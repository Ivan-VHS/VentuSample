#include "mainwindow.hpp"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFrame>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QUrl>
#include <QWidget>
#include <QDir>
#include <QMetaObject>
#include <algorithm>

namespace {
QString q(const std::string& value) { return QString::fromUtf8(value.c_str()); }
std::filesystem::path pathFromQString(const QString& value) {
    const QByteArray bytes = value.toUtf8();
    const auto* first = reinterpret_cast<const char8_t*>(bytes.constData());
    return std::filesystem::path(std::u8string(first, first + bytes.size()));
}
QString pathToQString(const std::filesystem::path& value) {
#ifdef _WIN32
    return QString::fromStdWString(value.wstring());
#else
    const auto utf8 = value.u8string();
    return QString::fromUtf8(reinterpret_cast<const char*>(utf8.data()), static_cast<qsizetype>(utf8.size()));
#endif
}
QStringList styleNames() {
    return {"Boom Bap", "East Coast", "Harlem Dip-Trap", "G-Unit", "Modern Trap",
            "Atlanta Trap", "UK Drill", "UK Grime", "Memphis", "Jazz Rap", "G-Funk",
            "Boom Trap", "Cripple Trill"};
}
QStringList targetNames() { return {"Полный луп", "Мелодия", "Ударные", "Бас", "One-shot kit"}; }
QStringList scaleNames() { return {"Natural Minor", "Harmonic Minor", "Dorian", "Phrygian", "Major", "Minor Pentatonic"}; }
QStringList samplerNames() { return {"Clean", "MPC 2500", "MPC 3000XL", "MPC 5000"}; }
QStringList timbreNames() { return {"Keys", "Bell", "Pluck", "Organ", "Brass", "Strings"}; }
QStringList keyNames() { return {"C", "C♯", "D", "D♯", "E", "F", "F♯", "G", "G♯", "A", "A♯", "B"}; }
QStringList rateNames() { return {"22050", "44100", "48000", "96000"}; }

void setCombo(QComboBox* combo, const QString& value) {
    const int index = combo->findText(value, Qt::MatchFixedString);
    if (index >= 0) combo->setCurrentIndex(index);
}

QWidget* makeCard(const QString& title, QLayout* content) {
    auto* card = new QGroupBox(title);
    card->setLayout(content);
    return card;
}
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("VentuSample — Hip-Hop Sample Foundry");
    setMinimumSize(980, 720);
    resize(1320, 900);
    buildUi();
    loadSettings();
    applyTheme();
}

MainWindow::~MainWindow() {
    if (worker_.joinable()) worker_.join();
}

void MainWindow::addChoice(QComboBox* combo, const QStringList& labels, int selected) {
    combo->addItems(labels);
    const int lastIndex = labels.isEmpty() ? 0 : static_cast<int>(labels.size()) - 1;
    combo->setCurrentIndex(std::clamp(selected, 0, lastIndex));
    combo->setMinimumHeight(38);
}

QSlider* MainWindow::addSliderRow(QGridLayout* grid, int row, const QString& label,
                                   int min, int max, int value, QLabel** valueLabel) {
    auto* text = new QLabel(label);
    auto* slider = new QSlider(Qt::Horizontal);
    slider->setRange(min, max);
    slider->setValue(value);
    slider->setMinimumHeight(34);
    auto* valueText = new QLabel(QString::number(value));
    valueText->setMinimumWidth(48);
    valueText->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(text, row, 0);
    grid->addWidget(slider, row, 1);
    grid->addWidget(valueText, row, 2);
    *valueLabel = valueText;
    connect(slider, &QSlider::valueChanged, valueText, [valueText](int next) {
        valueText->setText(QString::number(next));
    });
    return slider;
}

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    auto* header = new QHBoxLayout();
    auto* brand = new QVBoxLayout();
    auto* title = new QLabel("VENTUSAMPLE");
    title->setObjectName("brandTitle");
    auto* subtitle = new QLabel("HIP-HOP SAMPLE FOUNDRY  /  NATIVE DSP WORKSTATION");
    subtitle->setObjectName("brandSubtitle");
    brand->addWidget(title);
    brand->addWidget(subtitle);
    header->addLayout(brand);
    header->addStretch();
    darkThemeCheck_ = new QCheckBox("Тёмная тема");
    darkThemeCheck_->setCursor(Qt::PointingHandCursor);
    header->addWidget(darkThemeCheck_, 0, Qt::AlignVCenter);
    root->addLayout(header);

    auto* hero = new QHBoxLayout();
    auto* heroText = new QVBoxLayout();
    auto* heroTitle = new QLabel("Собери собственный звук.");
    heroTitle->setObjectName("heroTitle");
    auto* heroSub = new QLabel("Генерируй лупы, мелодии, драм-паттерны, бас и наборы one-shots по точным параметрам.");
    heroSub->setObjectName("heroSubtitle");
    heroSub->setWordWrap(true);
    heroText->addWidget(heroTitle);
    heroText->addWidget(heroSub);
    hero->addLayout(heroText, 1);
    auto* badge = new QLabel("OFFLINE  •  WAV PCM16  •  SEEDED");
    badge->setObjectName("statusPill");
    badge->setAlignment(Qt::AlignCenter);
    hero->addWidget(badge, 0, Qt::AlignVCenter);
    root->addLayout(hero);

    auto* tabs = new QTabWidget();
    auto* studio = new QWidget();
    auto* studioLayout = new QVBoxLayout(studio);
    studioLayout->setContentsMargins(4, 10, 4, 4);

    auto* upper = new QHBoxLayout();
    upper->setSpacing(14);
    auto* musicalGrid = new QGridLayout();
    musicalGrid->setHorizontalSpacing(14);
    musicalGrid->setVerticalSpacing(10);
    styleBox_ = new QComboBox(); addChoice(styleBox_, styleNames(), 1);
    targetBox_ = new QComboBox(); addChoice(targetBox_, targetNames(), 0);
    keyBox_ = new QComboBox(); addChoice(keyBox_, keyNames(), 0);
    scaleBox_ = new QComboBox(); addChoice(scaleBox_, scaleNames(), 0);
    samplerBox_ = new QComboBox(); addChoice(samplerBox_, samplerNames(), 2);
    timbreBox_ = new QComboBox(); addChoice(timbreBox_, timbreNames(), 0);
    rateBox_ = new QComboBox(); addChoice(rateBox_, rateNames(), 1);

    const auto addField = [musicalGrid](int row, int col, const QString& label, QWidget* field) {
        auto* text = new QLabel(label);
        text->setObjectName("fieldLabel");
        musicalGrid->addWidget(text, row * 2, col);
        musicalGrid->addWidget(field, row * 2 + 1, col);
    };
    addField(0, 0, "ЖАНРОВЫЙ ПРОФИЛЬ", styleBox_);
    addField(0, 1, "ТИП РЕЗУЛЬТАТА", targetBox_);
    addField(1, 0, "ТОНАЛЬНОСТЬ", keyBox_);
    addField(1, 1, "ЛАД", scaleBox_);
    addField(2, 0, "СЭМПЛЕРНЫЙ ХАРАКТЕР", samplerBox_);
    addField(2, 1, "ТЕМБР МЕЛОДИИ", timbreBox_);
    addField(3, 0, "ЧАСТОТА ДИСКРЕТИЗАЦИИ", rateBox_);

    upper->addWidget(makeCard("01  /  SOUND IDENTITY", musicalGrid), 3);

    auto* structureGrid = new QGridLayout();
    structureGrid->setVerticalSpacing(10);
    bpmSpin_ = new QSpinBox(); bpmSpin_->setRange(60, 190); bpmSpin_->setValue(93); bpmSpin_->setSuffix(" BPM");
    barsSpin_ = new QSpinBox(); barsSpin_->setRange(1, 16); barsSpin_->setValue(4); barsSpin_->setSuffix(" bars");
    variationsSpin_ = new QSpinBox(); variationsSpin_->setRange(1, 32); variationsSpin_->setValue(8);
    seedSpin_ = new QSpinBox(); seedSpin_->setRange(0, 2147483647); seedSpin_->setValue(2500);
    snareOffsetSpin_ = new QSpinBox(); snareOffsetSpin_->setRange(-30, 30); snareOffsetSpin_->setValue(15); snareOffsetSpin_->setSuffix(" ms");
    ratioSpin_ = new QDoubleSpinBox(); ratioSpin_->setRange(1.0, 12.0); ratioSpin_->setSingleStep(0.5); ratioSpin_->setValue(4.0); ratioSpin_->setSuffix(":1");
    const auto addSpin = [structureGrid](int row, int col, const QString& label, QWidget* field) {
        auto* text = new QLabel(label);
        text->setObjectName("fieldLabel");
        structureGrid->addWidget(text, row * 2, col);
        structureGrid->addWidget(field, row * 2 + 1, col);
    };
    addSpin(0, 0, "ТЕМП", bpmSpin_);
    addSpin(0, 1, "ДЛИНА", barsSpin_);
    addSpin(1, 0, "ВАРИАЦИИ", variationsSpin_);
    addSpin(1, 1, "SEED", seedSpin_);
    addSpin(2, 0, "СМЕЩЕНИЕ SNARE", snareOffsetSpin_);
    addSpin(2, 1, "BUS COMP", ratioSpin_);
    upper->addWidget(makeCard("02  /  GRID & STRUCTURE", structureGrid), 2);
    studioLayout->addLayout(upper);

    auto* detailRow = new QHBoxLayout();
    detailRow->setSpacing(14);
    auto* motionGrid = new QGridLayout();
    motionGrid->setVerticalSpacing(4);
    swingSlider_ = addSliderRow(motionGrid, 0, "Swing", 50, 68, 58, &swingValue_);
    densitySlider_ = addSliderRow(motionGrid, 1, "Плотность", 0, 100, 64, &densityValue_);
    gritSlider_ = addSliderRow(motionGrid, 2, "Sampler grit", 0, 100, 22, &gritValue_);
    saturationSlider_ = addSliderRow(motionGrid, 3, "Сатурация", 0, 100, 24, &saturationValue_);
    stereoSlider_ = addSliderRow(motionGrid, 4, "Стерео", 0, 100, 72, &stereoValue_);
    detailRow->addWidget(makeCard("03  /  GROOVE & TEXTURE", motionGrid), 1);

    auto* filterGrid = new QGridLayout();
    filterGrid->setVerticalSpacing(4);
    hpSlider_ = addSliderRow(filterGrid, 0, "High-pass", 20, 300, 32, &hpValue_);
    lpSlider_ = addSliderRow(filterGrid, 1, "Low-pass", 2000, 20000, 18000, &lpValue_);
    tripletCheck_ = new QCheckBox("Triplet hat rolls"); tripletCheck_->setChecked(true);
    humanizeCheck_ = new QCheckBox("Micro-humanize ±2 ms"); humanizeCheck_->setChecked(true);
    dustCheck_ = new QCheckBox("Vinyl / transient dust"); dustCheck_->setChecked(false);
    filterGrid->addWidget(tripletCheck_, 2, 0, 1, 2);
    filterGrid->addWidget(humanizeCheck_, 3, 0, 1, 2);
    filterGrid->addWidget(dustCheck_, 4, 0, 1, 2);
    detailRow->addWidget(makeCard("04  /  MASTER & MOTION", filterGrid), 1);
    studioLayout->addLayout(detailRow);

    auto* sourceGrid = new QGridLayout();
    sourceSamplePath_ = new QLineEdit();
    sourceSamplePath_->setPlaceholderText("Необязательно: исходный PCM16 WAV для chop-нарезки");
    auto* sourceBrowse = new QPushButton("Загрузить WAV…");
    sourceMixSlider_ = new QSlider(Qt::Horizontal);
    sourceMixSlider_->setRange(0, 100);
    sourceMixSlider_->setValue(65);
    sourceMixValue_ = new QLabel("65");
    sourceMixValue_->setMinimumWidth(35);
    sourcePitchSpin_ = new QSpinBox();
    sourcePitchSpin_->setRange(-12, 12);
    sourcePitchSpin_->setValue(0);
    sourcePitchSpin_->setSuffix(" st");
    sourceGrid->addWidget(sourceSamplePath_, 0, 0, 1, 3);
    sourceGrid->addWidget(sourceBrowse, 0, 3);
    sourceGrid->addWidget(new QLabel("SOURCE MIX"), 1, 0);
    sourceGrid->addWidget(sourceMixSlider_, 1, 1);
    sourceGrid->addWidget(sourceMixValue_, 1, 2);
    sourceGrid->addWidget(new QLabel("PITCH SHIFT"), 1, 3);
    sourceGrid->addWidget(sourcePitchSpin_, 1, 4);
    studioLayout->addWidget(makeCard("05  /  OPTIONAL SAMPLE-CHOP SOURCE", sourceGrid));

    auto* exportCardLayout = new QVBoxLayout();
    auto* pathRow = new QHBoxLayout();
    outputPath_ = new QLineEdit();
    outputPath_->setPlaceholderText("Папка экспорта WAV + manifest.json");
    auto* browseButton = new QPushButton("Выбрать папку…");
    pathRow->addWidget(outputPath_, 1);
    pathRow->addWidget(browseButton);
    exportCardLayout->addLayout(pathRow);
    auto* actionRow = new QHBoxLayout();
    generateButton_ = new QPushButton("СОЗДАТЬ SAMPLE PACK");
    generateButton_->setObjectName("primaryButton");
    generateButton_->setMinimumHeight(48);
    openFolderButton_ = new QPushButton("Открыть папку");
    openFolderButton_->setEnabled(false);
    actionRow->addWidget(generateButton_, 1);
    actionRow->addWidget(openFolderButton_);
    exportCardLayout->addLayout(actionRow);
    progress_ = new QProgressBar();
    progress_->setRange(0, 100);
    progress_->setValue(0);
    progress_->setTextVisible(false);
    progress_->setFixedHeight(8);
    exportCardLayout->addWidget(progress_);
    statusLabel_ = new QLabel("Готов к генерации · локальная обработка · без сетевых запросов");
    statusLabel_->setObjectName("mutedText");
    exportCardLayout->addWidget(statusLabel_);
    studioLayout->addWidget(makeCard("06  /  EXPORT STATION", exportCardLayout));

    auto* studioScroll = new QScrollArea();
    studioScroll->setWidgetResizable(true);
    studioScroll->setFrameShape(QFrame::NoFrame);
    studioScroll->setWidget(studio);
    tabs->addTab(studioScroll, "Генератор");

    auto* outputTab = new QWidget();
    auto* outputLayout = new QVBoxLayout(outputTab);
    summaryLabel_ = new QLabel("Здесь появятся созданные WAV-файлы и параметры последней сессии.");
    summaryLabel_->setObjectName("mutedText");
    filesList_ = new QListWidget();
    outputLayout->addWidget(summaryLabel_);
    outputLayout->addWidget(filesList_, 2);
    log_ = new QPlainTextEdit();
    log_->setReadOnly(true);
    log_->setMaximumBlockCount(1000);
    log_->setPlaceholderText("Журнал генерации и диагностики…");
    outputLayout->addWidget(log_, 1);
    tabs->addTab(outputTab, "Результаты и журнал");
    connect(filesList_, &QListWidget::itemDoubleClicked, this, [](QListWidgetItem* item) {
        const QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });

    auto* about = new QWidget();
    auto* aboutLayout = new QVBoxLayout(about);
    auto* aboutText = new QLabel(
        "<h2>VentuSample Native DSP</h2>"
        "<p>Процедурный генератор хип-хоп-сэмплов с воспроизводимыми seed-вариациями."
        " Все вычисления выполняются локально, WAV экспортируется в PCM16.</p>"
        "<h3>Профили</h3><p>Boom Bap · East Coast · Harlem Dip-Trap · G-Unit · Modern Trap · Atlanta Trap · UK Drill · UK Grime · Memphis · Jazz Rap · G-Funk · Boom Trap · Cripple Trill.</p>"
        "<h3>Что генерируется</h3><p>Полные лупы, мелодии, ударные, басовые партии и multi-one-shot kit."
        " Каждая вариация получает независимый детерминированный seed.</p>"
        "<h3>Инженерные ограничения</h3><p>Офлайн-рендеринг; фиксированные диапазоны параметров; ограничение длительности и числа вариантов;"
        " отсутствие скрытой загрузки библиотек, сетевого телеметрирования и обращения к устройствам записи.</p>"
        "<p>0.1.0 · Windows / Linux / macOS (Qt 6 desktop target)</p>");
    aboutText->setWordWrap(true);
    aboutLayout->addWidget(aboutText);
    aboutLayout->addStretch();
    tabs->addTab(about, "О приложении");

    root->addWidget(tabs, 1);
    setCentralWidget(central);

    connect(browseButton, &QPushButton::clicked, this, [this] {
        const QString initial = outputPath_->text().isEmpty()
            ? QStandardPaths::writableLocation(QStandardPaths::MusicLocation) : outputPath_->text();
        const QString directory = QFileDialog::getExistingDirectory(this, "Папка экспорта", initial);
        if (!directory.isEmpty()) outputPath_->setText(directory);
    });
    connect(sourceBrowse, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(this, "Выбрать исходный WAV",
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
            "WAV PCM16 (*.wav *.WAV)");
        if (!path.isEmpty()) sourceSamplePath_->setText(path);
    });
    connect(sourceMixSlider_, &QSlider::valueChanged, this, [this](int value) {
        sourceMixValue_->setText(QString::number(value));
    });
    connect(rateBox_, &QComboBox::currentTextChanged, this, [this](const QString& text) {
        const int rate = text.toInt();
        const int maximum = std::min(20000, static_cast<int>(rate * 0.49));
        lpSlider_->setMaximum(maximum);
        if (lpSlider_->value() > maximum) lpSlider_->setValue(maximum);
    });
    connect(generateButton_, &QPushButton::clicked, this, &MainWindow::startGeneration);
    connect(openFolderButton_, &QPushButton::clicked, this, [this] {
        const QString path = lastOutputDirectory_.isEmpty() ? outputPath_->text() : lastOutputDirectory_;
        if (!path.isEmpty()) QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });
    connect(darkThemeCheck_, &QCheckBox::toggled, this, [this] { applyTheme(); saveSettings(); });
    connect(swingSlider_, &QSlider::valueChanged, this, [this](int) { saveSettings(); });
}

void MainWindow::loadSettings() {
    QSettings settings;
    settings.beginGroup("generator");
    setCombo(styleBox_, settings.value("style", "East Coast").toString());
    setCombo(targetBox_, settings.value("target", "Полный луп").toString());
    setCombo(keyBox_, settings.value("key", "C").toString());
    setCombo(scaleBox_, settings.value("scale", "Natural Minor").toString());
    setCombo(samplerBox_, settings.value("sampler", "MPC 3000XL").toString());
    setCombo(timbreBox_, settings.value("timbre", "Keys").toString());
    setCombo(rateBox_, settings.value("rate", "44100").toString());
    bpmSpin_->setValue(settings.value("bpm", 93).toInt());
    barsSpin_->setValue(settings.value("bars", 4).toInt());
    variationsSpin_->setValue(settings.value("variations", 8).toInt());
    seedSpin_->setValue(settings.value("seed", 2500).toInt());
    snareOffsetSpin_->setValue(settings.value("snare_offset", 15).toInt());
    ratioSpin_->setValue(settings.value("ratio", 4.0).toDouble());
    swingSlider_->setValue(settings.value("swing", 58).toInt());
    densitySlider_->setValue(settings.value("density", 64).toInt());
    gritSlider_->setValue(settings.value("grit", 22).toInt());
    saturationSlider_->setValue(settings.value("saturation", 24).toInt());
    stereoSlider_->setValue(settings.value("stereo", 72).toInt());
    hpSlider_->setValue(settings.value("hp", 32).toInt());
    lpSlider_->setValue(settings.value("lp", 18000).toInt());
    tripletCheck_->setChecked(settings.value("triplets", true).toBool());
    humanizeCheck_->setChecked(settings.value("humanize", true).toBool());
    dustCheck_->setChecked(settings.value("dust", false).toBool());
    darkThemeCheck_->setChecked(settings.value("dark", false).toBool());
    outputPath_->setText(settings.value("output", QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/VentuSample").toString());
    sourceSamplePath_->setText(settings.value("source_sample", "").toString());
    sourceMixSlider_->setValue(settings.value("source_mix", 65).toInt());
    sourcePitchSpin_->setValue(settings.value("source_pitch", 0).toInt());
    settings.endGroup();
}

void MainWindow::saveSettings() {
    QSettings settings;
    settings.beginGroup("generator");
    settings.setValue("style", styleBox_->currentText());
    settings.setValue("target", targetBox_->currentText());
    settings.setValue("key", keyBox_->currentText());
    settings.setValue("scale", scaleBox_->currentText());
    settings.setValue("sampler", samplerBox_->currentText());
    settings.setValue("timbre", timbreBox_->currentText());
    settings.setValue("rate", rateBox_->currentText());
    settings.setValue("bpm", bpmSpin_->value());
    settings.setValue("bars", barsSpin_->value());
    settings.setValue("variations", variationsSpin_->value());
    settings.setValue("seed", seedSpin_->value());
    settings.setValue("snare_offset", snareOffsetSpin_->value());
    settings.setValue("ratio", ratioSpin_->value());
    settings.setValue("swing", swingSlider_->value());
    settings.setValue("density", densitySlider_->value());
    settings.setValue("grit", gritSlider_->value());
    settings.setValue("saturation", saturationSlider_->value());
    settings.setValue("stereo", stereoSlider_->value());
    settings.setValue("hp", hpSlider_->value());
    settings.setValue("lp", lpSlider_->value());
    settings.setValue("triplets", tripletCheck_->isChecked());
    settings.setValue("humanize", humanizeCheck_->isChecked());
    settings.setValue("dust", dustCheck_->isChecked());
    settings.setValue("dark", darkThemeCheck_->isChecked());
    settings.setValue("output", outputPath_->text());
    settings.setValue("source_sample", sourceSamplePath_->text());
    settings.setValue("source_mix", sourceMixSlider_->value());
    settings.setValue("source_pitch", sourcePitchSpin_->value());
    settings.endGroup();
    settings.sync();
}

void MainWindow::applyTheme() {
    const bool dark = darkThemeCheck_->isChecked();
    if (dark) {
        setStyleSheet(R"QSS(
            QMainWindow, QWidget { background: #071A2D; color: #E8EDF2; }
            QGroupBox { background: #0F2A45; border: 1px solid #31516F; border-left: 3px solid #FF7B86; border-radius: 12px; margin-top: 14px; padding: 18px 12px 12px; font-weight: 650; }
            QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 5px; color: #9FC3F3; }
            QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QListWidget, QPlainTextEdit { min-height: 36px; background: #0A223B; border: 1px solid #31516F; border-radius: 7px; padding: 8px; selection-background-color: #244F82; }
            QPushButton { background: #173A5E; border: 1px solid #31516F; border-radius: 8px; padding: 10px 16px; min-height: 24px; color: #E8EDF2; }
            QPushButton:hover { border-color: #8AB6FF; background: #204B75; }
            QPushButton:disabled { color: #7C91A8; background: #102840; }
            QPushButton#primaryButton { background: #70A5FF; color: #071A2D; border: none; font-weight: 750; letter-spacing: 1px; }
            QPushButton#primaryButton:hover { background: #8AB6FF; }
            QSlider::groove:horizontal { height: 5px; background: #31516F; border-radius: 2px; }
            QSlider::sub-page:horizontal { background: #70A5FF; border-radius: 2px; }
            QSlider::handle:horizontal { background: #E8EDF2; width: 15px; margin: -6px 0; border-radius: 7px; }
            QProgressBar { background: #173A5E; border: 0; border-radius: 4px; }
            QProgressBar::chunk { background: #70A5FF; border-radius: 4px; }
            QLabel#brandTitle { color: #FFFFFF; font-size: 22px; font-weight: 850; letter-spacing: 3px; }
            QLabel#brandSubtitle, QLabel#fieldLabel { color: #9FB4C9; font-size: 10px; font-weight: 700; letter-spacing: 1px; }
            QLabel#heroTitle { font-size: 23px; font-weight: 750; color: #FFFFFF; }
            QLabel#heroSubtitle, QLabel#mutedText { color: #B3C4D6; }
            QLabel#statusPill { color: #8AB6FF; background: #0F2A45; border: 1px solid #31516F; border-radius: 12px; padding: 10px 14px; font-size: 10px; font-weight: 800; }
            QTabBar::tab { background: #0F2A45; color: #B3C4D6; border: 1px solid #31516F; padding: 11px 18px; margin-right: 5px; border-top-left-radius: 7px; border-top-right-radius: 7px; }
            QTabBar::tab:selected { color: #FFFFFF; background: #173A5E; border-bottom: 3px solid #FF7B86; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator { width: 17px; height: 17px; }
        )QSS");
    } else {
        setStyleSheet(R"QSS(
            QMainWindow, QWidget { background: #F4F8FD; color: #122C45; }
            QGroupBox { background: #FFFFFF; border: 1px solid #CAD8E8; border-left: 3px solid #E62E2D; border-radius: 12px; margin-top: 14px; padding: 18px 12px 12px; font-weight: 650; }
            QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 5px; color: #004E98; }
            QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QListWidget, QPlainTextEdit { min-height: 36px; background: #FFFFFF; border: 1px solid #CAD8E8; border-radius: 7px; padding: 8px; selection-background-color: #D9E8FF; }
            QPushButton { background: #EAF1FA; border: 1px solid #CAD8E8; border-radius: 8px; padding: 10px 16px; min-height: 24px; color: #004E98; }
            QPushButton:hover { border-color: #1A5FDB; background: #D9E8FF; }
            QPushButton:disabled { color: #73869A; background: #EDF2F8; }
            QPushButton#primaryButton { background: #004E98; color: #FFFFFF; border: none; font-weight: 750; letter-spacing: 1px; }
            QPushButton#primaryButton:hover { background: #1A5FDB; }
            QSlider::groove:horizontal { height: 5px; background: #CAD8E8; border-radius: 2px; }
            QSlider::sub-page:horizontal { background: #004E98; border-radius: 2px; }
            QSlider::handle:horizontal { background: #FFFFFF; border: 2px solid #004E98; width: 15px; margin: -6px 0; border-radius: 7px; }
            QProgressBar { background: #EAF1FA; border: 0; border-radius: 4px; }
            QProgressBar::chunk { background: #004E98; border-radius: 4px; }
            QLabel#brandTitle { color: #004E98; font-size: 22px; font-weight: 850; letter-spacing: 3px; }
            QLabel#brandSubtitle, QLabel#fieldLabel { color: #4D6177; font-size: 10px; font-weight: 700; letter-spacing: 1px; }
            QLabel#heroTitle { font-size: 23px; font-weight: 750; color: #122C45; }
            QLabel#heroSubtitle, QLabel#mutedText { color: #4D6177; }
            QLabel#statusPill { color: #004E98; background: #FFFFFF; border: 1px solid #CAD8E8; border-radius: 12px; padding: 10px 14px; font-size: 10px; font-weight: 800; }
            QTabBar::tab { background: #EAF1FA; color: #314B66; border: 1px solid #CAD8E8; padding: 11px 18px; margin-right: 5px; border-top-left-radius: 7px; border-top-right-radius: 7px; }
            QTabBar::tab:selected { color: #004E98; background: #FFFFFF; border-bottom: 3px solid #E62E2D; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator { width: 17px; height: 17px; }
        )QSS");
    }
}

ventu::Parameters MainWindow::parameters() const {
    ventu::Parameters p;
    p.style = static_cast<ventu::Style>(styleBox_->currentIndex());
    p.target = static_cast<ventu::Target>(targetBox_->currentIndex());
    p.rootNote = keyBox_->currentIndex();
    p.scale = static_cast<ventu::Scale>(scaleBox_->currentIndex());
    p.sampler = static_cast<ventu::Sampler>(samplerBox_->currentIndex());
    p.timbre = static_cast<ventu::Timbre>(timbreBox_->currentIndex());
    p.bpm = bpmSpin_->value();
    p.bars = barsSpin_->value();
    p.variations = variationsSpin_->value();
    p.seed = seedSpin_->value();
    p.swingPercent = swingSlider_->value();
    p.densityPercent = densitySlider_->value();
    p.gritPercent = gritSlider_->value();
    p.saturationPercent = saturationSlider_->value();
    p.stereoPercent = stereoSlider_->value();
    p.highPassHz = hpSlider_->value();
    p.lowPassHz = lpSlider_->value();
    p.snareOffsetMs = snareOffsetSpin_->value();
    p.sampleRate = rateBox_->currentText().toInt();
    p.sourceSamplePath = sourceSamplePath_->text().trimmed().toUtf8().toStdString();
    p.sourceMixPercent = sourceMixSlider_->value();
    p.sourcePitchSemitones = sourcePitchSpin_->value();
    p.busCompressionRatio = ratioSpin_->value();
    p.tripletRolls = tripletCheck_->isChecked();
    p.microHumanize = humanizeCheck_->isChecked();
    p.vinylDust = dustCheck_->isChecked();
    return p;
}

void MainWindow::setBusy(bool busy) {
    generateButton_->setEnabled(!busy);
    generateButton_->setText(busy ? "РЕНДЕРИНГ SAMPLE PACK…" : "СОЗДАТЬ SAMPLE PACK");
    progress_->setRange(0, busy ? 0 : 100);
    if (!busy) progress_->setValue(100);
    statusLabel_->setText(busy ? "Рендеринг выполняется в рабочем потоке. Интерфейс остаётся отзывчивым." : statusLabel_->text());
}

void MainWindow::appendLog(const QString& message) {
    log_->appendPlainText(message);
}

void MainWindow::startGeneration() {
    if (worker_.joinable()) worker_.join();
    const auto p = parameters();
    const auto issue = ventu::validate(p);
    if (!issue.empty()) {
        QMessageBox::warning(this, "Проверь параметры", q(issue));
        return;
    }
    QString destination = outputPath_->text().trimmed();
    if (destination.isEmpty()) {
        QMessageBox::warning(this, "Нет папки экспорта", "Выбери папку, куда сохранить WAV и manifest.");
        return;
    }
    const QString canonical = QDir::cleanPath(destination);
    const auto reply = QMessageBox::question(this, "Создать sample pack",
        QString("Сгенерировать %1 вариаций профиля %2 и сохранить в:\n%3 ?")
            .arg(p.variations).arg(q(ventu::toString(p.style)), canonical),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
    if (reply != QMessageBox::Yes) return;

    saveSettings();
    filesList_->clear();
    lastOutputDirectory_ = canonical;
    openFolderButton_->setEnabled(false);
    setBusy(true);
    appendLog(QString("START  %1 BPM · %2 bars · %3 variants · seed %4")
        .arg(p.bpm).arg(p.bars).arg(p.variations).arg(p.seed));
    appendLog(QString("PROFILE  %1 / %2 / %3 / %4 Hz")
        .arg(q(ventu::toString(p.style)), q(ventu::toString(p.target)), q(ventu::toString(p.sampler)))
        .arg(p.sampleRate));

    try {
        worker_ = std::thread([this, p, canonical] {
            ventu::BatchResult result;
            try {
                result = ventu::generatePack(p, pathFromQString(canonical));
            } catch (const std::exception& exception) {
                result.error = std::string("Необработанная ошибка рендеринга: ") + exception.what();
            } catch (...) {
                result.error = "Неизвестная ошибка рендеринга.";
            }
            QStringList files;
            for (const auto& file : result.files) files.push_back(pathToQString(file));
            const QString error = QString::fromUtf8(result.error.c_str());
            QMetaObject::invokeMethod(this, [this, files, error, p] {
            if (worker_.joinable()) worker_.join();
            setBusy(false);
            if (!error.isEmpty()) {
                statusLabel_->setText("Ошибка экспорта — подробности в журнале.");
                appendLog("ERROR  " + error);
                QMessageBox::critical(this, "Ошибка генерации", error);
                return;
            }
            for (const auto& path : files) {
                auto* item = new QListWidgetItem(QFileInfo(path).fileName(), filesList_);
                item->setData(Qt::UserRole, path);
            }
            const QString manifest = QDir(lastOutputDirectory_).filePath("ventusample_manifest.json");
            auto* manifestItem = new QListWidgetItem(QFileInfo(manifest).fileName(), filesList_);
            manifestItem->setData(Qt::UserRole, manifest);
            summaryLabel_->setText(QString("Создано файлов: %1 WAV + manifest · %2 BPM · %3 · seed %4 · peak ceiling −0.3 dBFS")
                .arg(files.size()).arg(p.bpm).arg(q(ventu::toString(p.style))).arg(p.seed));
            statusLabel_->setText(QString("Готово · %1 аудиофайлов сохранено локально.").arg(files.size()));
            appendLog(QString("DONE  %1 audio files + ventusample_manifest.json").arg(files.size()));
                openFolderButton_->setEnabled(true);
            }, Qt::QueuedConnection);
        });
    } catch (const std::exception& exception) {
        setBusy(false);
        const QString message = QString::fromUtf8(exception.what());
        appendLog("THREAD ERROR  " + message);
        QMessageBox::critical(this, "Не удалось запустить генератор", message);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    if (worker_.joinable()) worker_.join();
    event->accept();
}