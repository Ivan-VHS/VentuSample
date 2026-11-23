#include "quantum_audio_lab.hpp"

#include "quantum_audio.hpp"

#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QSizePolicy>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QObject>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <cmath>
#include <string>
#include <vector>

namespace ventu {
namespace {
QString q(std::string_view value) {
    return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
}

std::filesystem::path nativePath(const QString& value) {
#ifdef _WIN32
    return std::filesystem::path(value.toStdWString());
#else
    const auto utf8 = value.toUtf8();
    return std::filesystem::path(std::string(utf8.constData(),
        static_cast<std::size_t>(utf8.size())));
#endif
}

class SignalScope final : public QWidget {
public:
    explicit SignalScope(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(92);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setBuffer(const AudioBuffer& audio) {
        peaks_.clear();
        const auto channels = audio.channels > 0 ? static_cast<std::size_t>(audio.channels) : 0u;
        if (channels == 0u || audio.samples.empty()) {
            update();
            return;
        }
        const auto frames = audio.samples.size() / channels;
        constexpr std::size_t points = 512u;
        peaks_.reserve(points);
        for (std::size_t point = 0; point < points; ++point) {
            const auto first = point * frames / points;
            const auto last = std::max(first + 1u, (point + 1u) * frames / points);
            double peak = 0.0;
            for (std::size_t frame = first; frame < std::min(last, frames); ++frame) {
                for (std::size_t ch = 0; ch < channels; ++ch) {
                    const double sample = audio.samples[frame * channels + ch];
                    if (std::isfinite(sample)) peak = std::max(peak, std::abs(sample));
                }
            }
            peaks_.push_back(static_cast<float>(std::clamp(peak, 0.0, 1.0)));
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRectF area = rect().adjusted(1.0, 1.0, -1.0, -1.0);
        painter.fillRect(area, palette().base());
        painter.setPen(QPen(palette().mid().color(), 1.0, Qt::DashLine));
        for (int i = 1; i < 4; ++i) {
            const double y = area.top() + area.height() * static_cast<double>(i) / 4.0;
            painter.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
        }
        const double centre = area.center().y();
        painter.setPen(QPen(palette().mid().color(), 1.0));
        painter.drawLine(QPointF(area.left(), centre), QPointF(area.right(), centre));
        if (peaks_.empty()) return;
        const QColor accent = palette().highlight().color();
        painter.setPen(QPen(accent, 1.25));
        const double dx = area.width() / static_cast<double>(peaks_.size());
        for (std::size_t i = 0; i < peaks_.size(); ++i) {
            const double x = area.left() + (static_cast<double>(i) + 0.5) * dx;
            const double height = static_cast<double>(peaks_[i]) * area.height() * 0.46;
            painter.drawLine(QPointF(x, centre - height), QPointF(x, centre + height));
        }
    }

private:
    std::vector<float> peaks_;
};

using ChainStep = QuantumAudioChainStep;

void setControl(QDoubleSpinBox* control, double value) {
    control->setValue(std::clamp(value, 0.0, 1.0));
}

void showStats(QLabel* label, const AudioBuffer& audio, const QString& prefix) {
    const auto stats = measure(audio);
    const double peakDb = stats.peak > 0.0 ? 20.0 * std::log10(stats.peak) : -120.0;
    const double rmsDb = stats.rms > 0.0 ? 20.0 * std::log10(stats.rms) : -120.0;
    label->setText(QString("%1  ·  %2 Hz  ·  %3 ch  ·  peak %4 dBFS  ·  RMS %5 dBFS  ·  crest %6 dB")
        .arg(prefix)
        .arg(audio.sampleRate)
        .arg(audio.channels)
        .arg(peakDb, 0, 'f', 1)
        .arg(rmsDb, 0, 'f', 1)
        .arg(stats.crestDb, 0, 'f', 1));
}
} // namespace

void showQuantumAudioLab(QWidget* parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Ventu — Quantum Audio Engine 133");
    dialog.resize(1160, 780);
    dialog.setMinimumSize(960, 650);

    auto* root = new QVBoxLayout(&dialog);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(10);

    auto* heading = new QLabel("QUANTUM AUDIO ENGINE  /  133 DSP MODULES");
    QFont headingFont = heading->font();
    headingFont.setBold(true);
    headingFont.setPointSize(15);
    heading->setFont(headingFont);
    root->addWidget(heading);

    auto* description = new QLabel(
        "Пять семейств обработки · недеструктивная цепочка · локальный офлайн-рендеринг WAV. "
        "Физические концепции здесь реализованы как воспроизводимые DSP-модели.");
    description->setWordWrap(true);
    root->addWidget(description);

    auto* fileRow = new QHBoxLayout();
    auto* openButton = new QPushButton("Открыть WAV…");
    auto* resetButton = new QPushButton("Сбросить обработку");
    auto* exportButton = new QPushButton("Экспортировать WAV…");
    fileRow->addWidget(openButton);
    fileRow->addWidget(resetButton);
    fileRow->addStretch();
    fileRow->addWidget(exportButton);
    root->addLayout(fileRow);

    auto* scope = new SignalScope();
    scope->setToolTip("Пиковая огибающая обработанного сигнала");
    root->addWidget(scope);

    auto* status = new QLabel("Открой WAV PCM16, выбери модули и собери цепочку обработки.");
    status->setWordWrap(true);
    root->addWidget(status);

    auto* body = new QHBoxLayout();
    auto* browser = new QVBoxLayout();
    auto* category = new QComboBox();
    category->addItems({
        "Все семейства", "Quarks & Hadron", "Quantum Mechanics",
        "Atoms & Orbitals", "Fields & Relativity", "Experimental Instruments"
    });
    auto* catalogue = new QListWidget();
    catalogue->setMinimumWidth(290);
    browser->addWidget(new QLabel("КАТАЛОГ МОДУЛЕЙ"));
    browser->addWidget(category);
    browser->addWidget(catalogue, 1);
    body->addLayout(browser, 1);

    auto* inspector = new QVBoxLayout();
    auto* moduleInfo = new QPlainTextEdit();
    moduleInfo->setReadOnly(true);
    moduleInfo->setMaximumHeight(210);
    inspector->addWidget(new QLabel("ОПИСАНИЕ И ВИЗУАЛЬНАЯ МОДЕЛЬ"));
    inspector->addWidget(moduleInfo);

    auto* controlsBox = new QWidget();
    auto* controls = new QFormLayout(controlsBox);
    auto* amount = new QDoubleSpinBox();
    auto* tone = new QDoubleSpinBox();
    auto* rate = new QDoubleSpinBox();
    auto* mix = new QDoubleSpinBox();
    for (auto* spin : {amount, tone, rate, mix}) {
        spin->setRange(0.0, 1.0);
        spin->setDecimals(2);
        spin->setSingleStep(0.05);
        spin->setMinimumWidth(110);
    }
    controls->addRow("Интенсивность", amount);
    controls->addRow("Тон / частота", tone);
    controls->addRow("Скорость / время", rate);
    controls->addRow("Wet / Dry", mix);
    inspector->addWidget(controlsBox);

    auto* chainTitle = new QLabel("ЦЕПОЧКА ОБРАБОТКИ");
    inspector->addWidget(chainTitle);
    auto* chainList = new QListWidget();
    chainList->setMinimumHeight(130);
    inspector->addWidget(chainList, 1);

    auto* chainButtons = new QHBoxLayout();
    auto* addButton = new QPushButton("Добавить выбранный");
    auto* removeButton = new QPushButton("Убрать");
    auto* clearButton = new QPushButton("Очистить цепочку");
    chainButtons->addWidget(addButton);
    chainButtons->addWidget(removeButton);
    chainButtons->addWidget(clearButton);
    inspector->addLayout(chainButtons);

    auto* renderButton = new QPushButton("ОБРАБОТАТЬ ЦЕПОЧКУ");
    renderButton->setObjectName("primaryButton");
    inspector->addWidget(renderButton);
    body->addLayout(inspector, 2);
    root->addLayout(body, 1);

    auto* closeRow = new QHBoxLayout();
    closeRow->addStretch();
    auto* closeButton = new QPushButton("Закрыть");
    closeRow->addWidget(closeButton);
    root->addLayout(closeRow);

    AudioBuffer original;
    AudioBuffer rendered;
    bool hasAudio = false;
    std::vector<ChainStep> chain;

    const auto refreshCatalogue = [&] {
        const QString wanted = category->currentText();
        const QString selectedId = catalogue->currentItem()
            ? catalogue->currentItem()->data(Qt::UserRole).toString() : QString{};
        catalogue->clear();
        int selectedRow = -1;
        int row = 0;
        for (const auto& plugin : quantumAudioPlugins()) {
            if (wanted != "Все семейства" && q(quantumAudioCategoryName(plugin.category)) != wanted)
                continue;
            auto* item = new QListWidgetItem(q(plugin.name), catalogue);
            item->setData(Qt::UserRole, q(plugin.id));
            if (q(plugin.id) == selectedId) selectedRow = row;
            ++row;
        }
        if (catalogue->count() > 0) catalogue->setCurrentRow(selectedRow >= 0 ? selectedRow : 0);
    };

    const auto selectedPlugin = [&]() -> const QuantumAudioPlugin* {
        const auto* item = catalogue->currentItem();
        return item ? findQuantumAudioPlugin(item->data(Qt::UserRole).toString().toStdString()) : nullptr;
    };

    const auto storeControls = [&] {
        const auto* plugin = selectedPlugin();
        if (!plugin) return;
        QSettings settings("Ventu", "QuantumAudioEngine");
        settings.beginGroup("modules");
        settings.beginGroup(QString::fromUtf8(plugin->id.data(), static_cast<qsizetype>(plugin->id.size())));
        settings.setValue("amount", amount->value());
        settings.setValue("tone", tone->value());
        settings.setValue("rate", rate->value());
        settings.setValue("mix", mix->value());
        settings.endGroup();
        settings.endGroup();
    };

    const auto loadControls = [&] {
        const auto* plugin = selectedPlugin();
        if (!plugin) {
            moduleInfo->clear();
            addButton->setEnabled(false);
            return;
        }
        setControl(amount, plugin->defaults.amount);
        setControl(tone, plugin->defaults.tone);
        setControl(rate, plugin->defaults.rate);
        setControl(mix, plugin->defaults.mix);
        QSettings settings("Ventu", "QuantumAudioEngine");
        settings.beginGroup("modules");
        settings.beginGroup(QString::fromUtf8(plugin->id.data(), static_cast<qsizetype>(plugin->id.size())));
        if (settings.contains("amount")) amount->setValue(settings.value("amount").toDouble());
        if (settings.contains("tone")) tone->setValue(settings.value("tone").toDouble());
        if (settings.contains("rate")) rate->setValue(settings.value("rate").toDouble());
        if (settings.contains("mix")) mix->setValue(settings.value("mix").toDouble());
        settings.endGroup();
        settings.endGroup();

        const QString text = QString("MODULE: %1\nID: %2\nFAMILY: %3\nDSP MODEL: %4\n\n%5\n\nVISUAL MODEL:\n%6")
            .arg(q(plugin->name), q(plugin->id), q(quantumAudioCategoryName(plugin->category)),
                 q(quantumDspModelName(plugin->model)), q(plugin->description), q(plugin->visual));
        moduleInfo->setPlainText(text);
        addButton->setEnabled(true);
    };

    const auto saveChain = [&] {
        QSettings settings("Ventu", "QuantumAudioEngine");
        QStringList saved;
        for (const auto& step : chain) {
            saved.push_back(QStringList{
                QString::fromStdString(step.id),
                QString::number(step.parameters.amount, 'g', 12),
                QString::number(step.parameters.tone, 'g', 12),
                QString::number(step.parameters.rate, 'g', 12),
                QString::number(step.parameters.mix, 'g', 12)
            }.join('|'));
        }
        settings.setValue("chain/steps", saved);
    };

    const auto restoreChain = [&] {
        QSettings settings("Ventu", "QuantumAudioEngine");
        const auto saved = settings.value("chain/steps").toStringList();
        for (const auto& row : saved) {
            const auto fields = row.split('|');
            if (fields.size() != 5 || findQuantumAudioPlugin(fields[0].toStdString()) == nullptr)
                continue;
            bool okAmount = false, okTone = false, okRate = false, okMix = false;
            const double savedAmount = fields[1].toDouble(&okAmount);
            const double savedTone = fields[2].toDouble(&okTone);
            const double savedRate = fields[3].toDouble(&okRate);
            const double savedMix = fields[4].toDouble(&okMix);
            if (!okAmount || !okTone || !okRate || !okMix
                || !std::isfinite(savedAmount) || !std::isfinite(savedTone)
                || !std::isfinite(savedRate) || !std::isfinite(savedMix)
                || savedAmount < 0.0 || savedAmount > 1.0
                || savedTone < 0.0 || savedTone > 1.0
                || savedRate < 0.0 || savedRate > 1.0
                || savedMix < 0.0 || savedMix > 1.0)
                continue;
            if (chain.size() >= quantumAudioPlugins().size()) break;
            chain.push_back({fields[0].toStdString(),
                {savedAmount, savedTone, savedRate, savedMix}});
        }
    };

    const auto refreshChain = [&] {
        chainList->clear();
        for (const auto& step : chain) {
            const auto* plugin = findQuantumAudioPlugin(step.id);
            if (!plugin) continue;
            auto* item = new QListWidgetItem(
                q(plugin->name) + QString("   ·   wet %1%").arg(step.parameters.mix * 100.0, 0, 'f', 0),
                chainList);
            item->setData(Qt::UserRole, QString::fromStdString(step.id));
        }
        removeButton->setEnabled(!chain.empty());
        clearButton->setEnabled(!chain.empty());
        renderButton->setEnabled(hasAudio);
    };

    QObject::connect(category, &QComboBox::currentTextChanged, &dialog, [&](const QString&) {
        refreshCatalogue();
    });
    QObject::connect(catalogue, &QListWidget::currentRowChanged, &dialog, [&](int) {
        loadControls();
    });
    QObject::connect(addButton, &QPushButton::clicked, &dialog, [&] {
        const auto* plugin = selectedPlugin();
        if (!plugin) return;
        storeControls();
        if (chain.size() >= quantumAudioPlugins().size()) {
            QMessageBox::information(&dialog, "Лимит цепочки", "В цепочку уже добавлены все 133 модуля.");
            return;
        }
        chain.push_back({std::string(plugin->id), {amount->value(), tone->value(), rate->value(), mix->value()}});
        saveChain();
        refreshChain();
        status->setText(QString("В цепочку добавлен %1 · модулей: %2")
            .arg(q(plugin->name)).arg(chain.size()));
    });
    QObject::connect(removeButton, &QPushButton::clicked, &dialog, [&] {
        const int row = chainList->currentRow();
        if (row < 0 || static_cast<std::size_t>(row) >= chain.size()) return;
        chain.erase(chain.begin() + row);
        saveChain();
        refreshChain();
    });
    QObject::connect(clearButton, &QPushButton::clicked, &dialog, [&] {
        chain.clear();
        saveChain();
        refreshChain();
    });

    QObject::connect(openButton, &QPushButton::clicked, &dialog, [&] {
        const QString path = QFileDialog::getOpenFileName(&dialog, "Открыть исходный WAV", {},
            "WAV audio (*.wav *.WAV)");
        if (path.isEmpty()) return;
        std::string error;
        AudioBuffer loaded = loadWave(nativePath(path), error);
        if (!error.empty() || loaded.samples.empty()) {
            QMessageBox::warning(&dialog, "Не удалось открыть WAV",
                QString::fromStdString(error.empty() ? "В файле нет PCM-аудиоданных." : error));
            return;
        }
        if (loaded.channels == 1) {
            std::vector<float> stereo;
            stereo.reserve(loaded.samples.size() * 2u);
            for (const float sample : loaded.samples) {
                stereo.push_back(sample);
                stereo.push_back(sample);
            }
            loaded.samples = std::move(stereo);
            loaded.channels = 2;
        }
        original = std::move(loaded);
        rendered = original;
        hasAudio = true;
        scope->setBuffer(rendered);
        showStats(status, rendered, "Загружено");
        resetButton->setEnabled(true);
        exportButton->setEnabled(true);
        renderButton->setEnabled(true);
    });

    QObject::connect(renderButton, &QPushButton::clicked, &dialog, [&] {
        if (!hasAudio) return;
        AudioBuffer candidate = original;
        const auto result = processQuantumAudioChain(chain, candidate);
        if (!result.success) {
            QMessageBox::warning(&dialog, "DSP-обработка остановлена",
                QString::fromStdString(result.message));
            return;
        }
        rendered = std::move(candidate);
        scope->setBuffer(rendered);
        showStats(status, rendered, QString("Обработано · %1 модулей").arg(result.completedSteps));
    });

    QObject::connect(resetButton, &QPushButton::clicked, &dialog, [&] {
        if (!hasAudio) return;
        rendered = original;
        scope->setBuffer(rendered);
        showStats(status, rendered, "Исходный сигнал");
    });

    QObject::connect(exportButton, &QPushButton::clicked, &dialog, [&] {
        if (!hasAudio) return;
        QString path = QFileDialog::getSaveFileName(&dialog, "Экспортировать обработанный WAV", {},
            "WAV PCM16 (*.wav)");
        if (path.isEmpty()) return;
        if (!path.endsWith(".wav", Qt::CaseInsensitive)) path += ".wav";
        if (QFileInfo::exists(path)) {
            const auto answer = QMessageBox::question(&dialog, "Перезаписать файл?",
                "Файл уже существует. Заменить его?",
                QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
            if (answer != QMessageBox::Yes) return;
        }
        std::string error;
        if (!writeWave(nativePath(path), rendered, error)) {
            QMessageBox::critical(&dialog, "Ошибка экспорта", QString::fromStdString(error));
            return;
        }
        status->setText("Экспорт завершён: " + path);
    });

    QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    resetButton->setEnabled(false);
    exportButton->setEnabled(false);
    addButton->setEnabled(false);
    removeButton->setEnabled(false);
    clearButton->setEnabled(false);
    renderButton->setEnabled(false);
    restoreChain();
    refreshCatalogue();
    loadControls();
    refreshChain();
    dialog.exec();
}

} // namespace ventu
