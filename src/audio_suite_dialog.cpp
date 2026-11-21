#include "audio_suite_dialog.hpp"

#include <QFileDialog>
#include <QFileInfo>
#include <QByteArray>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <utility>

AudioSuiteDialog::AudioSuiteDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Ventu Audio Suite — 66 DSP effects");
    setMinimumSize(1040, 700);
    resize(1180, 780);

    auto* root = new QVBoxLayout(this);
    auto* heading = new QLabel("VENTU AUDIO SUITE");
    heading->setStyleSheet("font-size:20px;font-weight:800;letter-spacing:2px;");
    auto* subtitle = new QLabel("66 ВСТРОЕННЫХ DSP-ЭФФЕКТОВ  ·  ОФЛАЙН-СТЕРЕО  ·  PCM16 WAV");
    subtitle->setStyleSheet("color:#6b91c7;font-size:10px;font-weight:700;");
    root->addWidget(heading);
    root->addWidget(subtitle);

    auto* columns = new QHBoxLayout();
    library_ = new QListWidget(this);
    library_->setMinimumWidth(285);
    library_->setAlternatingRowColors(true);
    auto* center = new QVBoxLayout();
    description_ = new QLabel("Выбери эффект, чтобы увидеть назначение и визуальную концепцию интерфейса.");
    description_->setWordWrap(true);
    description_->setMinimumHeight(82);
    description_->setStyleSheet("padding:12px;border:1px solid #31517b;border-radius:5px;");
    center->addWidget(description_);

    auto* controls = new QFormLayout();
    const auto makeSlider = [this, controls](const QString& name, QSlider*& slider, int initial) {
        auto* row = new QWidget(this);
        auto* layout = new QHBoxLayout(row);
        layout->setContentsMargins(0,0,0,0);
        slider = new QSlider(Qt::Horizontal, row);
        slider->setRange(0,100);
        slider->setValue(initial);
        auto* value = new QLabel(QString::number(initial), row);
        value->setMinimumWidth(34);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        layout->addWidget(slider,1);
        layout->addWidget(value);
        QObject::connect(slider, &QSlider::valueChanged, row, [value](int v) {
            value->setText(QString::number(v));
        });
        QObject::connect(slider, &QSlider::valueChanged, this, [this](int) {
            updateSelectedParameters();
        });
        controls->addRow(name, row);
    };
    makeSlider("Amount / глубина", amount_, 55);
    makeSlider("Wet / обработанный сигнал", wet_, 35);
    makeSlider("Rate / скорость", rate_, 25);
    makeSlider("Tone / тон", tone_, 55);
    makeSlider("Width / ширина", width_, 50);
    auto* controlBox = new QWidget(this);
    controlBox->setLayout(controls);
    center->addWidget(controlBox);

    auto* rackColumn = new QVBoxLayout();
    auto* rackTitle = new QLabel("ЦЕПОЧКА ЭФФЕКТОВ");
    rackTitle->setStyleSheet("font-weight:800;letter-spacing:1px;");
    rackList_ = new QListWidget(this);
    rackList_->setMinimumWidth(265);
    rackColumn->addWidget(rackTitle);
    rackColumn->addWidget(rackList_,1);

    columns->addWidget(library_, 3);
    columns->addLayout(center, 4);
    columns->addLayout(rackColumn, 3);
    root->addLayout(columns,1);

    auto* actions = new QHBoxLayout();
    importButton_ = new QPushButton("Загрузить WAV…", this);
    addButton_ = new QPushButton("Добавить эффект →", this);
    removeButton_ = new QPushButton("Удалить из цепочки", this);
    renderButton_ = new QPushButton("ОБРАБОТАТЬ И ЭКСПОРТИРОВАТЬ", this);
    renderButton_->setStyleSheet("font-weight:800;background:#174ea6;color:white;padding:10px;");
    actions->addWidget(importButton_);
    actions->addWidget(addButton_);
    actions->addWidget(removeButton_);
    actions->addStretch();
    actions->addWidget(renderButton_);
    root->addLayout(actions);

    sourceLabel_ = new QLabel("Источник: WAV не загружен. Поддерживается PCM16 mono/stereo.");
    sourceLabel_->setWordWrap(true);
    status_ = new QLabel("Эффекты работают офлайн; цепочка применяется слева направо.");
    status_->setWordWrap(true);
    root->addWidget(sourceLabel_);
    root->addWidget(status_);

    for (const auto& plugin : ventu::audioPluginCatalog()) {
        auto* item = new QListWidgetItem(QString::fromStdString(plugin.name), library_);
        item->setData(Qt::UserRole, QString::fromStdString(plugin.id));
        item->setToolTip(QString::fromStdString(plugin.category + " — " + plugin.description));
    }
    library_->setCurrentRow(0);

    connect(library_, &QListWidget::currentRowChanged, this, [this](int) { showLibraryDetails(); });
    connect(rackList_, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && static_cast<std::size_t>(row) < rack_.size()) loadSelectedParameters();
    });
    connect(importButton_, &QPushButton::clicked, this, [this] { loadInput(); });
    connect(addButton_, &QPushButton::clicked, this, [this] { addSelectedPlugin(); });
    connect(removeButton_, &QPushButton::clicked, this, [this] { removeSelectedPlugin(); });
    connect(renderButton_, &QPushButton::clicked, this, [this] { exportProcessedRack(); });
    showLibraryDetails();
    refreshRack();
}

AudioSuiteDialog::~AudioSuiteDialog() {
    if (worker_.joinable()) worker_.join();
}

void AudioSuiteDialog::showLibraryDetails() {
    const auto* item = library_->currentItem();
    if (!item) return;
    const auto id = item->data(Qt::UserRole).toString().toStdString();
    const auto* plugin = ventu::findAudioPlugin(id);
    if (!plugin) return;
    description_->setText(QString("%1\n%2\n\nИнтерфейс: %3\nID: %4  ·  Версия: %5")
        .arg(QString::fromStdString(plugin->category),
             QString::fromStdString(plugin->description),
             QString::fromStdString(plugin->interfaceConcept),
             QString::fromStdString(plugin->id),
             QString::fromStdString(plugin->version)));
}

ventu::AudioPluginParameters AudioSuiteDialog::parametersFromControls() const {
    return {amount_->value()/100.0f, wet_->value()/100.0f, rate_->value()/100.0f,
            tone_->value()/100.0f, width_->value()/100.0f};
}

void AudioSuiteDialog::setParameterControls(const ventu::AudioPluginParameters& p) {
    const QSignalBlocker a(amount_), w(wet_), r(rate_), t(tone_), d(width_);
    amount_->setValue(static_cast<int>(std::lround(p.amount*100.0f)));
    wet_->setValue(static_cast<int>(std::lround(p.wet*100.0f)));
    rate_->setValue(static_cast<int>(std::lround(p.rate*100.0f)));
    tone_->setValue(static_cast<int>(std::lround(p.tone*100.0f)));
    width_->setValue(static_cast<int>(std::lround(p.width*100.0f)));
}

void AudioSuiteDialog::refreshRack() {
    const int selected = rackList_->currentRow();
    rackList_->clear();
    for (const auto& entry : rack_) {
        const auto* descriptor = ventu::findAudioPlugin(entry.pluginId);
        auto* item = new QListWidgetItem(descriptor ? QString::fromStdString(descriptor->name)
                                                    : QString::fromStdString(entry.pluginId), rackList_);
        item->setData(Qt::UserRole, QString::fromStdString(entry.pluginId));
    }
    if (!rack_.empty()) {
        const int row = std::clamp(selected, 0, static_cast<int>(rack_.size())-1);
        rackList_->setCurrentRow(row);
        loadSelectedParameters();
    }
    removeButton_->setEnabled(!rack_.empty());
    renderButton_->setEnabled(!rack_.empty() && !sourceBuffer_.left.empty());
}

void AudioSuiteDialog::loadSelectedParameters() {
    const int row = rackList_->currentRow();
    if (row < 0 || static_cast<std::size_t>(row) >= rack_.size()) return;
    setParameterControls(rack_[static_cast<std::size_t>(row)].parameters);
}

void AudioSuiteDialog::updateSelectedParameters() {
    const int row = rackList_ ? rackList_->currentRow() : -1;
    if (row < 0 || static_cast<std::size_t>(row) >= rack_.size()) return;
    rack_[static_cast<std::size_t>(row)].parameters = parametersFromControls();
}

void AudioSuiteDialog::loadInput() {
    const QString file = QFileDialog::getOpenFileName(this, "Загрузить исходный WAV", {},
                                                       "WAV audio (*.wav);;All files (*.*)");
    if (file.isEmpty()) return;
    ventu::AudioBuffer loaded;
    std::string error;
    if (!ventu::loadPcm16Wav(pathFromQString(file), loaded, error)) {
        QMessageBox::warning(this, "Не удалось открыть WAV", QString::fromStdString(error));
        return;
    }
    sourcePath_ = pathFromQString(file);
    sourceBuffer_ = std::move(loaded);
    const double seconds = static_cast<double>(sourceBuffer_.frames())/sourceBuffer_.sampleRate;
    sourceLabel_->setText(QString("Источник: %1  ·  %2 Hz  ·  PCM16 → stereo  ·  %3 с  ·  %4 кадров")
        .arg(QFileInfo(file).fileName())
        .arg(sourceBuffer_.sampleRate)
        .arg(seconds,0,'f',2)
        .arg(static_cast<qulonglong>(sourceBuffer_.frames())));
    status_->setText("WAV импортирован. Добавь эффекты в цепочку и запускай офлайн-обработку.");
    renderButton_->setEnabled(!rack_.empty());
}

void AudioSuiteDialog::addSelectedPlugin() {
    const auto* item = library_->currentItem();
    if (!item) return;
    const auto id = item->data(Qt::UserRole).toString().toStdString();
    if (!ventu::findAudioPlugin(id)) return;
    rack_.push_back({id, parametersFromControls()});
    refreshRack();
    rackList_->setCurrentRow(static_cast<int>(rack_.size())-1);
    status_->setText(QString("Добавлен %1. Цепочка: %2 эффект(ов).")
        .arg(item->text()).arg(rack_.size()));
}

void AudioSuiteDialog::removeSelectedPlugin() {
    const int row = rackList_->currentRow();
    if (row < 0 || static_cast<std::size_t>(row) >= rack_.size()) return;
    rack_.erase(rack_.begin()+row);
    refreshRack();
    status_->setText(QString("В цепочке осталось эффектов: %1.").arg(rack_.size()));
}

void AudioSuiteDialog::setProcessingEnabled(bool enabled) {
    importButton_->setEnabled(enabled);
    addButton_->setEnabled(enabled);
    removeButton_->setEnabled(enabled && !rack_.empty());
    renderButton_->setEnabled(enabled && !rack_.empty() && !sourceBuffer_.left.empty());
    library_->setEnabled(enabled);
    rackList_->setEnabled(enabled);
    amount_->setEnabled(enabled);
    wet_->setEnabled(enabled);
    rate_->setEnabled(enabled);
    tone_->setEnabled(enabled);
    width_->setEnabled(enabled);
}

void AudioSuiteDialog::exportProcessedRack() {
    if (worker_.joinable()) worker_.join();
    updateSelectedParameters();
    if (sourceBuffer_.left.empty() || rack_.empty()) return;
    const std::filesystem::path defaultPath = sourcePath_.empty()
        ? std::filesystem::path("Ventu_processed.wav")
        : sourcePath_.parent_path() / std::filesystem::path(sourcePath_.stem().u8string()+u8"_ventu_fx.wav");
    const QString destination = QFileDialog::getSaveFileName(this, "Экспорт обработанного WAV",
        pathToQString(defaultPath), "WAV audio (*.wav)");
    if (destination.isEmpty()) return;
    const auto source = sourceBuffer_;
    const auto chain = rack_;
    const std::filesystem::path output = pathFromQString(destination);
    setProcessingEnabled(false);
    status_->setText("Обработка выполняется в рабочем потоке. Исходный WAV не изменяется.");
    worker_ = std::thread([this, source, chain, output] () mutable {
        auto rendered = source;
        std::string error;
        const bool processed = ventu::processAudioChain(rendered, chain, error);
        const bool saved = processed && ventu::savePcm16Wav(output, rendered, error);
        QMetaObject::invokeMethod(this, [this, saved, error, output] {
            setProcessingEnabled(true);
            if (!saved) {
                status_->setText("Обработка не завершена.");
                QMessageBox::critical(this, "Ошибка DSP / экспорта", QString::fromStdString(error));
                return;
            }
            status_->setText(QString("Готово: %1").arg(pathToQString(output)));
            QMessageBox::information(this, "Ventu Audio Suite", "Обработанный PCM16 WAV экспортирован.");
        }, Qt::QueuedConnection);
    });
}
