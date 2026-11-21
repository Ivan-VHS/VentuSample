#include "cineforge_dialog.hpp"
#include "cineforge.hpp"

#include <QByteArray>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMetaObject>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <filesystem>

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
}

CineForgeDialog::CineForgeDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("AETHER-ANAS // CINEFORGE QUANTUM");
    setMinimumSize(620, 520);
    resize(760, 600);

    auto* root = new QVBoxLayout(this);
    auto* title = new QLabel("CINEFORGE  /  HOLLYWOOD STEM ENGINE", this);
    title->setStyleSheet("font-family:Georgia;font-size:20px;font-weight:800;color:#0039A6;");
    auto* subtitle = new QLabel(
        "OFFLINE SCENE PACK GENERATOR  ·  DETERMINISTIC SEED  ·  DRY / WET / STEM EXPORT", this);
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("font-size:10px;font-weight:700;color:#0B1F3A;");
    root->addWidget(title);
    root->addWidget(subtitle);

    auto* form = new QFormLayout();
    prompt_ = new QLineEdit("Dark industrial thriller");
    prompt_->setMaxLength(512);
    prompt_->setPlaceholderText("Опиши сцену (сохраняется в manifest как metadata)");
    bpm_ = new QSpinBox(); bpm_->setRange(60, 190); bpm_->setValue(92); bpm_->setSuffix(" BPM");
    bars_ = new QSpinBox(); bars_->setRange(1, 16); bars_->setValue(4);
    packSize_ = new QSpinBox(); packSize_->setRange(8, 12); packSize_->setValue(10);
    seed_ = new QSpinBox(); seed_->setRange(0, 2147483647); seed_->setValue(920501);
    key_ = new QComboBox();
    key_->addItems({"C minor","C# minor","D minor","D# minor","E minor","F minor",
                    "F# minor","G minor","G# minor","A minor","A# minor","B minor"});
    key_->setCurrentIndex(5);
    sampleRate_ = new QComboBox(); sampleRate_->addItems({"48000","96000"});
    encoding_ = new QComboBox(); encoding_->addItems({"24-bit PCM","32-bit float"});
    form->addRow("SCENE PROMPT", prompt_);
    form->addRow("TEMPO", bpm_);
    form->addRow("MUSICAL KEY", key_);
    form->addRow("LENGTH / BARS", bars_);
    form->addRow("PACK SIZE", packSize_);
    form->addRow("SEED", seed_);
    form->addRow("SAMPLE RATE", sampleRate_);
    form->addRow("WAV ENCODING", encoding_);
    root->addLayout(form);

    auto* outputRow = new QHBoxLayout();
    output_ = new QLineEdit();
    output_->setPlaceholderText("Локальная папка, в которой будет создан новый пак");
    auto* browse = new QPushButton("Выбрать…");
    outputRow->addWidget(output_, 1);
    outputRow->addWidget(browse);
    root->addLayout(outputRow);

    auto* note = new QLabel(
        "На каждый объект создаются пять WAV: Dry, Wet, Attack, Body и Tail. "
        "Также сохраняются preview и cineforge_manifest.json. "
        "Scene prompt — metadata для текущего процедурного движка, не neural text-to-audio.", this);
    note->setWordWrap(true);
    note->setStyleSheet("padding:10px;border:1px solid #0039A6;border-radius:3px;");
    root->addWidget(note);

    status_ = new QLabel("Готово. Существующие папки паков не перезаписываются.", this);
    status_->setWordWrap(true);
    root->addWidget(status_);

    auto* actions = new QHBoxLayout();
    generateButton_ = new QPushButton("GENERATE CINEFORGE PACK");
    generateButton_->setStyleSheet("background:#B51A10;color:white;font-weight:800;padding:11px;");
    auto* close = new QPushButton("Закрыть");
    actions->addStretch();
    actions->addWidget(generateButton_);
    actions->addWidget(close);
    root->addLayout(actions);

    connect(browse, &QPushButton::clicked, this, [this] { chooseOutput(); });
    connect(generateButton_, &QPushButton::clicked, this, [this] { generate(); });
    connect(close, &QPushButton::clicked, this, &QDialog::accept);
}

CineForgeDialog::~CineForgeDialog() {
    if (worker_.joinable()) worker_.join();
}

void CineForgeDialog::chooseOutput() {
    const QString directory = QFileDialog::getExistingDirectory(
        this, "Папка экспорта CINEFORGE", output_->text());
    if (!directory.isEmpty()) output_->setText(directory);
}

void CineForgeDialog::generate() {
    if (worker_.joinable()) worker_.join();
    const auto destination = output_->text().trimmed();
    if (destination.isEmpty()) {
        QMessageBox::warning(this, "CINEFORGE", "Выбери локальную папку для экспорта.");
        return;
    }

    ventu::CineForgeSettings settings;
    const QByteArray promptBytes = prompt_->text().toUtf8();
    settings.scenePrompt.assign(promptBytes.constData(), static_cast<std::size_t>(promptBytes.size()));
    settings.bpm = bpm_->value();
    settings.rootNote = key_->currentIndex();
    settings.bars = bars_->value();
    settings.packSize = packSize_->value();
    settings.seed = static_cast<std::uint64_t>(seed_->value());
    settings.sampleRate = sampleRate_->currentText().toInt();
    settings.encoding = encoding_->currentIndex() == 0
        ? ventu::CineForgeEncoding::Pcm24 : ventu::CineForgeEncoding::Float32;
    settings.outputDirectory = pathFromQString(destination);

    generateButton_->setEnabled(false);
    status_->setText("Создание согласованного пака выполняется локально…");
    worker_ = std::thread([this, settings] {
        ventu::CineForgeResult result;
        const bool success = ventu::generateCineForgePack(settings, result);
        QMetaObject::invokeMethod(this, [this, success, result = std::move(result)] {
            generateButton_->setEnabled(true);
            if (!success) {
                status_->setText("Генерация не завершена: " + QString::fromStdString(result.error));
                QMessageBox::critical(this, "CINEFORGE export error",
                                      QString::fromStdString(result.error));
                return;
            }
            status_->setText(QString("ГОТОВО · %1 объектов · %2 WAV-файлов стемов · %3")
                .arg(static_cast<qulonglong>(result.assets.size())).arg(static_cast<qulonglong>(result.assets.size() * 5u))
                .arg(pathToQString(result.packDirectory)));
            QMessageBox::information(this, "CINEFORGE",
                QString("Sample Pack создан.\n\nПапка: %1\nManifest: %2\nPreview: %3")
                    .arg(pathToQString(result.packDirectory),
                         pathToQString(result.manifestPath),
                         pathToQString(result.previewPath)));
        }, Qt::QueuedConnection);
    });
}
