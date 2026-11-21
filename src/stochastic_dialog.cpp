#include "stochastic_dialog.hpp"

#include "audio_suite.hpp"

#include <QByteArray>
#include <QColor>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>
#include <filesystem>
#include <utility>

namespace {
std::filesystem::path pathFromQString(const QString& value) {
#ifdef _WIN32
    return std::filesystem::path(value.toStdWString());
#else
    const QByteArray bytes = value.toUtf8();
    return std::filesystem::u8path(std::string(bytes.constData(), static_cast<std::size_t>(bytes.size())));
#endif
}
QString pathToQString(const std::filesystem::path& value) {
#ifdef _WIN32
    return QString::fromStdWString(value.wstring());
#else
    const auto text = value.u8string();
    return QString::fromUtf8(reinterpret_cast<const char*>(text.data()), static_cast<qsizetype>(text.size()));
#endif
}
const std::vector<ventu::StochasticSlot>* slotsFor(const ventu::StochasticPlan& plan) {
    if (!plan.serial.empty()) return &plan.serial;
    if (!plan.branches.empty()) return &plan.branches.front();
    return nullptr;
}
}

StochasticDialog::StochasticDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("AETHER-ANAS // STOCHASTIC ROUTING META-HOST");
    setMinimumSize(780, 650);
    resize(920, 720);

    auto* root = new QVBoxLayout(this);
    auto* title = new QLabel("STOCHASTIC ROUTING ENGINE");
    title->setStyleSheet("font-family:Georgia;font-size:20px;font-weight:800;color:#0039A6;");
    auto* subtitle = new QLabel(
        "SEEDED CHAIN MUTATION  ·  ROUND-ROBIN  ·  PARALLEL SPLIT  ·  LOCKED SLOTS  ·  OFFLINE SAFETY");
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size:10px;font-weight:700;color:#0B1F3A;");
    root->addWidget(title);
    root->addWidget(subtitle);

    auto* form = new QFormLayout();
    mode_ = new QComboBox();
    mode_->addItems({"Micro-Variation", "Round-Robin Chain", "Chaos One-Shot"});
    branches_ = new QComboBox();
    branches_->addItem("Serial chain", 0);
    branches_->addItem("2 parallel branches", 2);
    branches_->addItem("3 parallel branches", 3);
    branches_->addItem("4 parallel branches", 4);
    seed_ = new QSpinBox(); seed_->setRange(0, 2147483647); seed_->setValue(920501);
    minPlugins_ = new QSpinBox(); minPlugins_->setRange(1, 12); minPlugins_->setValue(2);
    maxPlugins_ = new QSpinBox(); maxPlugins_->setRange(1, 12); maxPlugins_->setValue(5);
    roundRobinCount_ = new QSpinBox(); roundRobinCount_->setRange(4, 8); roundRobinCount_->setValue(6);
    form->addRow("MUTATION MODE", mode_);
    form->addRow("ROUTING TOPOLOGY", branches_);
    form->addRow("SEED", seed_);
    form->addRow("MIN PLUGINS", minPlugins_);
    form->addRow("MAX PLUGINS", maxPlugins_);
    form->addRow("ROUND-ROBIN PLANS", roundRobinCount_);
    root->addLayout(form);

    auto* inputRow = new QHBoxLayout();
    sourcePath_ = new QLineEdit();
    sourcePath_->setReadOnly(true);
    sourcePath_->setPlaceholderText("Загрузи локальный PCM16/PCM24/Float32 WAV для офлайн-рендера");
    auto* import = new QPushButton("Импорт WAV…");
    inputRow->addWidget(sourcePath_, 1);
    inputRow->addWidget(import);
    root->addLayout(inputRow);

    planSelector_ = new QComboBox();
    slots_ = new QListWidget();
    auto* slotActions = new QHBoxLayout();
    lockButton_ = new QPushButton("LOCK / UNLOCK SLOT");
    renderButton_ = new QPushButton("RENDER + EXPORT SELECTED CHAIN");
    renderButton_->setStyleSheet("background:#B51A10;color:white;font-weight:800;padding:10px;");
    slotActions->addWidget(lockButton_);
    slotActions->addStretch();
    slotActions->addWidget(renderButton_);
    root->addWidget(new QLabel("PRECOMPUTED CHAIN"));
    root->addWidget(planSelector_);
    root->addWidget(slots_, 1);
    root->addLayout(slotActions);

    status_ = new QLabel(
        "Генерация детерминирована Seed. Для рендера нужен локальный PCM16/PCM24/Float32 WAV; исходник не перезаписывается.");
    status_->setWordWrap(true);
    status_->setStyleSheet("padding:8px;border:1px solid #0039A6;border-radius:3px;");
    root->addWidget(status_);

    auto* actions = new QHBoxLayout();
    auto* build = new QPushButton("BUILD / MUTATE CHAIN");
    build->setStyleSheet("background:#0039A6;color:white;font-weight:800;padding:10px;");
    auto* close = new QPushButton("Закрыть");
    actions->addWidget(build);
    actions->addStretch();
    actions->addWidget(close);
    root->addLayout(actions);

    connect(import, &QPushButton::clicked, this, [this] { importSource(); });
    connect(build, &QPushButton::clicked, this, [this] { buildPlans(); });
    connect(lockButton_, &QPushButton::clicked, this, [this] { toggleSelectedLock(); });
    connect(renderButton_, &QPushButton::clicked, this, [this] { renderSelectedPlan(); });
    connect(planSelector_, &QComboBox::currentTextChanged, this, [this](const QString&) { updatePlanView(); });
    connect(close, &QPushButton::clicked, this, &QDialog::accept);

    lockButton_->setEnabled(false);
    renderButton_->setEnabled(false);
}

StochasticDialog::~StochasticDialog() {
    if (worker_.joinable()) worker_.join();
}

void StochasticDialog::importSource() {
    const QString file = QFileDialog::getOpenFileName(
        this, "Import source WAV (PCM16/24 or Float32)", {}, "WAV audio (*.wav);;All files (*.*)");
    if (file.isEmpty()) return;
    ventu::AudioSuiteBuffer loaded;
    std::string error;
    if (!ventu::loadWav(pathFromQString(file), loaded, error)) {
        QMessageBox::warning(this, "WAV import failed", QString::fromStdString(error));
        return;
    }
    source_ = std::move(loaded);
    sourcePath_->setText(file);
    status_->setText(QString("SOURCE READY · %1 frames · %2 Hz · Stereo Float32 in memory")
        .arg(static_cast<qulonglong>(source_.frames())).arg(source_.sampleRate));
    renderButton_->setEnabled(!plans_.empty());
}

void StochasticDialog::buildPlans() {
    ventu::StochasticConfig config;
    config.seed = static_cast<std::uint64_t>(seed_->value());
    config.minPlugins = static_cast<std::size_t>(minPlugins_->value());
    config.maxPlugins = static_cast<std::size_t>(maxPlugins_->value());
    config.roundRobinCount = static_cast<std::size_t>(roundRobinCount_->value());
    config.parallelBranches = static_cast<std::size_t>(branches_->currentData().toInt());
    config.mode = static_cast<ventu::MutationMode>(mode_->currentIndex());

    std::vector<ventu::StochasticPlan> next;
    std::string error;
    if (!ventu::buildStochasticPlans(config, currentSlots_, next, error)) {
        QMessageBox::warning(this, "Could not build stochastic chain", QString::fromStdString(error));
        return;
    }

    plans_ = std::move(next);
    planSelector_->blockSignals(true);
    planSelector_->clear();
    for (std::size_t i = 0; i < plans_.size(); ++i)
        planSelector_->addItem(QString("PLAN %1 · SEED %2").arg(static_cast<qulonglong>(i + 1u))
                                   .arg(static_cast<qulonglong>(plans_[i].seed)));
    planSelector_->setCurrentIndex(0);
    planSelector_->blockSignals(false);
    updatePlanView();
    status_->setText(QString("BUILT %1 deterministic plan(s). Plugin pool: all 66 native effects. Seed: %2.")
        .arg(static_cast<qulonglong>(plans_.size())).arg(seed_->value()));
    renderButton_->setEnabled(!source_.left.empty());
}

void StochasticDialog::updatePlanView() {
    slots_->clear();
    const int index = planSelector_->currentIndex();
    if (index < 0 || static_cast<std::size_t>(index) >= plans_.size()) {
        lockButton_->setEnabled(false);
        renderButton_->setEnabled(false);
        return;
    }
    const auto& plan = plans_[static_cast<std::size_t>(index)];
    const auto* current = slotsFor(plan);
    if (!current) return;

    for (std::size_t i = 0; i < current->size(); ++i) {
        const auto& slot = (*current)[i];
        auto* item = new QListWidgetItem(
            QString("SLOT %1  ·  %2  ·  AMT %3%  WET %4%  RATE %5%  %6")
                .arg(i + 1)
                .arg(QString::fromStdString(slot.effect.pluginId))
                .arg(static_cast<int>(slot.effect.parameters.amount * 100.0f))
                .arg(static_cast<int>(slot.effect.parameters.wet * 100.0f))
                .arg(static_cast<int>(slot.effect.parameters.rate * 100.0f))
                .arg(slot.locked ? "LOCKED" : "MUTABLE"), slots_);
        item->setData(Qt::UserRole, static_cast<int>(i));
        if (slot.locked) item->setForeground(QColor("#B51A10"));
    }
    const bool serial = !plan.serial.empty();
    lockButton_->setEnabled(serial && !plan.serial.empty());
    renderButton_->setEnabled(!source_.left.empty());
    if (serial) currentSlots_ = plan.serial;
}

void StochasticDialog::toggleSelectedLock() {
    const int planIndex = planSelector_->currentIndex();
    const int row = slots_->currentRow();
    if (planIndex < 0 || row < 0 || static_cast<std::size_t>(planIndex) >= plans_.size()) return;
    auto& plan = plans_[static_cast<std::size_t>(planIndex)];
    if (plan.serial.empty() || static_cast<std::size_t>(row) >= plan.serial.size()) return;
    plan.serial[static_cast<std::size_t>(row)].locked = !plan.serial[static_cast<std::size_t>(row)].locked;
    currentSlots_ = plan.serial;
    updatePlanView();
    slots_->setCurrentRow(row);
    status_->setText(plan.serial[static_cast<std::size_t>(row)].locked
        ? "SLOT LOCKED · processor, position and parameters will survive the next mutation."
        : "SLOT UNLOCKED · the next mutation may replace the processor and parameters.");
}

void StochasticDialog::renderSelectedPlan() {
    if (worker_.joinable()) worker_.join();
    if (source_.left.empty()) {
        QMessageBox::information(this, "Stochastic Router", "Сначала импортируй PCM16/PCM24/Float32 WAV.");
        return;
    }
    const int index = planSelector_->currentIndex();
    if (index < 0 || static_cast<std::size_t>(index) >= plans_.size()) return;

    const QString destination = QFileDialog::getSaveFileName(
        this, "Export stochastic render", "Ventu_Stochastic.wav", "WAV audio (*.wav)");
    if (destination.isEmpty()) return;

    const auto source = source_;
    const auto plan = plans_[static_cast<std::size_t>(index)];
    ventu::StochasticConfig config;
    config.seed = static_cast<std::uint64_t>(seed_->value());
    config.minPlugins = static_cast<std::size_t>(minPlugins_->value());
    config.maxPlugins = static_cast<std::size_t>(maxPlugins_->value());
    config.roundRobinCount = static_cast<std::size_t>(roundRobinCount_->value());
    config.parallelBranches = static_cast<std::size_t>(branches_->currentData().toInt());
    config.mode = static_cast<ventu::MutationMode>(mode_->currentIndex());
    const auto output = pathFromQString(destination);
    renderButton_->setEnabled(false);
    status_->setText("Offline stochastic processing is running on a worker thread…");

    worker_ = std::thread([this, source, plan, config, output] () mutable {
        auto rendered = source;
        std::string error;
        const bool processed = ventu::processStochasticPlan(rendered, plan, config, error);
        const bool saved = processed && ventu::savePcm16Wav(output, rendered, error);
        QMetaObject::invokeMethod(this, [this, saved, error, output] {
            renderButton_->setEnabled(true);
            if (!saved) {
                status_->setText("RENDER FAILED · " + QString::fromStdString(error));
                QMessageBox::critical(this, "Stochastic render failed", QString::fromStdString(error));
                return;
            }
            status_->setText("RENDER COMPLETE · " + pathToQString(output));
            QMessageBox::information(this, "Stochastic Router",
                                     "Processed PCM16/PCM24/Float32 WAV exported. Source file was not modified.");
        }, Qt::QueuedConnection);
    });
}
