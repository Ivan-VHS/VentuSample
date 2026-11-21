#include "mainwindow.hpp"
#include "quantum_audio_lab.hpp"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QCoreApplication>
#include <QMenu>
#include <QMenuBar>
#include <QDoubleSpinBox>
#include <QFont>
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
#include <QDialog>
#include <QDialogButtonBox>
#include <QTimer>
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
#include <array>

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
            "Boom Trap", "Cripple Trill", "Billionaire Golden Era"};
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
    setWindowTitle("VentuSample — рабочая станция аудиопаков");
    setMinimumSize(980, 720);
    resize(1320, 900);
    buildUi();
    loadSettings();
    applyTheme();
    updateBeatSync();
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
    auto* pluginsMenu = menuBar()->addMenu("Плагины");
    pluginsMenu->addAction("Quantum Audio Engine — 133 DSP", this, [this] {
        ventu::showQuantumAudioLab(this);
    });
    auto* viewMenu = menuBar()->addMenu("Вид");
    viewMenu->addAction("Настройки движения…", this, &MainWindow::showMotionSettings);
    pluginsMenu->addSeparator();
    pluginsMenu->addAction("Менеджер плагинов…", this, [this] { showPlugins(); });
    pluginsMenu->addSeparator();
    for (const auto& plugin : pluginManager_.plugins()) {
        const QString id = QString::fromStdString(plugin.id);
        pluginsMenu->addAction(QString::fromStdString(plugin.name), this, [this, id] { showPlugins(id); });
    }

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(20, 18, 20, 18);
    root->setSpacing(12);

    auto* header = new QHBoxLayout();
    auto* brand = new QVBoxLayout();
    auto* title = new QLabel("VENTUSAMPLE");
    title->setFont(QFont("Georgia", 18, QFont::Bold));
    title->setObjectName("brandTitle");
    auto* subtitle = new QLabel("ХИП-ХОП-СЭМПЛЫ  /  ЛОКАЛЬНАЯ DSP-СТАНЦИЯ");
    subtitle->setObjectName("brandSubtitle");
    brand->addWidget(title);
    brand->addWidget(subtitle);
    header->addLayout(brand);
    header->addStretch();
    atmosphereBox_ = new QComboBox();
    addChoice(atmosphereBox_, {"Рассвет", "Закат", "Ночь"}, 0);
    atmosphereBox_->setToolTip("Атмосфера интерфейса: рассветная, закатная или ночная палитра");
    atmosphereBox_->setMinimumWidth(130);
    header->addWidget(atmosphereBox_, 0, Qt::AlignVCenter);
    beatIndicator_ = new QLabel("● BPM SYNC");
    beatIndicator_->setObjectName("beatIndicator");
    beatIndicator_->setVisible(false);
    beatIndicator_->setStyleSheet("QLabel#beatIndicator { background: #0039A6; color: #FFFFFF; border: 1px solid #0039A6; border-radius: 5px; padding: 5px 8px; }");
    header->addWidget(beatIndicator_, 0, Qt::AlignVCenter);
    root->addLayout(header);

    auto* hero = new QHBoxLayout();
    auto* heroText = new QVBoxLayout();
    auto* heroTitle = new QLabel("Собери собственный звук.");
    heroTitle->setFont(QFont("Georgia", 18, QFont::Bold));
    heroTitle->setObjectName("heroTitle");
    auto* heroSub = new QLabel("Создавайте лупы, мелодии, ударные паттерны, басовые партии и наборы одиночных сэмплов по точным параметрам.");
    heroSub->setObjectName("heroSubtitle");
    heroSub->setWordWrap(true);
    heroText->addWidget(heroTitle);
    heroText->addWidget(heroSub);
    hero->addLayout(heroText, 1);
    auto* badge = new QLabel("ЛОКАЛЬНО  •  WAV PCM16  •  ПОВТОРЯЕМОЕ ЗЕРНО");
    badge->setObjectName("statusPill");
    badge->setAlignment(Qt::AlignCenter);
    hero->addWidget(badge, 0, Qt::AlignVCenter);
    root->addLayout(hero);

    tabs_ = new QTabWidget();
    auto* tabs = tabs_;
    connect(tabs_, &QTabWidget::currentChanged, this, [this](int) {
        if (tabs_) ventu::ui_motion::fadeIn(tabs_->currentWidget(), motionPreferences_, 180);
    });
    beatTimer_ = new QTimer(this);
    beatTimer_->setTimerType(Qt::PreciseTimer);
    connect(beatTimer_, &QTimer::timeout, this, [this] {
        if (motionPreferences_.syncToBpm && !motionPreferences_.reduceMotion && !motionPreferences_.ecoVisuals)
            ventu::ui_motion::pulse(beatIndicator_, motionPreferences_, 70);
    });
    auto* studio = new QWidget();
    auto* studioLayout = new QVBoxLayout(studio);
    studioLayout->setContentsMargins(4, 10, 4, 4);

    auto* upper = new QHBoxLayout();
    upper->setSpacing(12);
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

    upper->addWidget(makeCard("01  /  ЗВУКОВОЙ ПРОФИЛЬ", musicalGrid), 3);

    auto* structureGrid = new QGridLayout();
    structureGrid->setVerticalSpacing(10);
    bpmSpin_ = new QSpinBox(); bpmSpin_->setRange(60, 190); bpmSpin_->setValue(93); bpmSpin_->setSuffix(" BPM");
    connect(bpmSpin_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) { updateBeatSync(); });
    barsSpin_ = new QSpinBox(); barsSpin_->setRange(1, 16); barsSpin_->setValue(4); barsSpin_->setSuffix(" тактов");
    variationsSpin_ = new QSpinBox(); variationsSpin_->setRange(1, 32); variationsSpin_->setValue(8);
    seedSpin_ = new QSpinBox(); seedSpin_->setRange(0, 2147483647); seedSpin_->setValue(2500);
    snareOffsetSpin_ = new QSpinBox(); snareOffsetSpin_->setRange(-30, 30); snareOffsetSpin_->setValue(15); snareOffsetSpin_->setSuffix(" мс");
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
    addSpin(1, 1, "ЗЕРНО ВАРИАЦИИ", seedSpin_);
    addSpin(2, 0, "СМЕЩЕНИЕ МАЛОГО БАРАБАНА", snareOffsetSpin_);
    addSpin(2, 1, "КОЭФФИЦИЕНТ СЖАТИЯ ШИНЫ", ratioSpin_);
    upper->addWidget(makeCard("02  /  СЕТКА И СТРУКТУРА", structureGrid), 2);
    studioLayout->addLayout(upper);

    auto* detailRow = new QHBoxLayout();
    detailRow->setSpacing(12);
    auto* motionGrid = new QGridLayout();
    motionGrid->setVerticalSpacing(4);
    swingSlider_ = addSliderRow(motionGrid, 0, "Свинг", 50, 68, 58, &swingValue_);
    densitySlider_ = addSliderRow(motionGrid, 1, "Плотность", 0, 100, 64, &densityValue_);
    gritSlider_ = addSliderRow(motionGrid, 2, "Зернистость сэмплера", 0, 100, 22, &gritValue_);
    saturationSlider_ = addSliderRow(motionGrid, 3, "Сатурация", 0, 100, 24, &saturationValue_);
    stereoSlider_ = addSliderRow(motionGrid, 4, "Стерео", 0, 100, 72, &stereoValue_);
    detailRow->addWidget(makeCard("03  /  РИТМ И ФАКТУРА", motionGrid), 1);

    auto* filterGrid = new QGridLayout();
    filterGrid->setVerticalSpacing(4);
    hpSlider_ = addSliderRow(filterGrid, 0, "Верхний фильтр", 20, 300, 32, &hpValue_);
    lpSlider_ = addSliderRow(filterGrid, 1, "Нижний фильтр", 2000, 20000, 18000, &lpValue_);
    tripletCheck_ = new QCheckBox("Тройные роллы хэтов"); tripletCheck_->setChecked(true);
    humanizeCheck_ = new QCheckBox("Микрогуманизация ±2 мс"); humanizeCheck_->setChecked(true);
    dustCheck_ = new QCheckBox("Виниловая фактура / шум транзиентов"); dustCheck_->setChecked(false);
    stochasticCheck_ = new QCheckBox("Включить стохастическую цепь эффектов");
    stochasticCheck_->setToolTip("Меняет DSP-цепочку при офлайн-экспорте WAV. Realtime-аудиопоток не используется.");
    stochasticIntensitySlider_ = addSliderRow(filterGrid, 6, "Интенсивность стохастики", 0, 100, 45, &stochasticIntensityValue_);
    stochasticTriggerBox_ = new QComboBox();
    addChoice(stochasticTriggerBox_, {"Весь файл", "Каждый такт", "Музыкальные транзиенты"}, 0);
    stochasticTopologyBox_ = new QComboBox();
    addChoice(stochasticTopologyBox_, {"Гибрид", "Последовательная", "Параллельная"}, 0);
    auto* stochasticPoolGroup = new QGroupBox("ПУЛ ДОСТУПНЫХ DSP-ЭФФЕКТОВ");
    auto* stochasticPoolLayout = new QGridLayout(stochasticPoolGroup);
    const std::array<QString, 8> stochasticEffectNames{
        "Low-pass", "High-pass", "Saturator", "Bitcrusher",
        "Delay", "Reverb", "Ring mod", "Stereo width"
    };
    for (std::size_t effect = 0; effect < stochasticPoolChecks_.size(); ++effect) {
        stochasticPoolChecks_[effect] = new QCheckBox(stochasticEffectNames[effect]);
        stochasticPoolChecks_[effect]->setChecked(true);
        stochasticPoolLayout->addWidget(stochasticPoolChecks_[effect],
            static_cast<int>(effect / 2u), static_cast<int>(effect % 2u));
    }
    filterGrid->addWidget(tripletCheck_, 2, 0, 1, 2);
    filterGrid->addWidget(humanizeCheck_, 3, 0, 1, 2);
    filterGrid->addWidget(dustCheck_, 4, 0, 1, 2);
    filterGrid->addWidget(stochasticCheck_, 5, 0, 1, 2);
    filterGrid->addWidget(new QLabel("ТРИГГЕР МУТАЦИИ"), 8, 0);
    filterGrid->addWidget(stochasticTriggerBox_, 8, 1, 1, 2);
    filterGrid->addWidget(stochasticPoolGroup, 9, 0, 1, 3);
    filterGrid->addWidget(new QLabel("ТОПОЛОГИЯ ЦЕПИ"), 10, 0);
    filterGrid->addWidget(stochasticTopologyBox_, 10, 1, 1, 2);
    detailRow->addWidget(makeCard("04  /  ОБРАБОТКА И ДВИЖЕНИЕ", filterGrid), 1);
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
    sourcePitchSpin_->setSuffix(" полутонов");
    sourceGrid->addWidget(sourceSamplePath_, 0, 0, 1, 3);
    sourceGrid->addWidget(sourceBrowse, 0, 3);
    sourceGrid->addWidget(new QLabel("ДОЛЯ ИСХОДНОГО СЭМПЛА"), 1, 0);
    sourceGrid->addWidget(sourceMixSlider_, 1, 1);
    sourceGrid->addWidget(sourceMixValue_, 1, 2);
    sourceGrid->addWidget(new QLabel("СДВИГ ВЫСОТЫ"), 1, 3);
    sourceGrid->addWidget(sourcePitchSpin_, 1, 4);
    studioLayout->addWidget(makeCard("05  /  ИСХОДНЫЙ СЭМПЛ ДЛЯ НАРЕЗКИ", sourceGrid));

    auto* exportCardLayout = new QVBoxLayout();
    auto* pathRow = new QHBoxLayout();
    outputPath_ = new QLineEdit();
    outputPath_->setPlaceholderText("Папка экспорта WAV + manifest.json");
    auto* browseButton = new QPushButton("Выбрать папку…");
    pathRow->addWidget(outputPath_, 1);
    pathRow->addWidget(browseButton);
    exportCardLayout->addLayout(pathRow);
    auto* tonalExportHint = new QLabel("Метки тональности и лада попадут в имена WAV и manifest (например, A_Minor или C_Major). У drum-only файлов тональность не указывается.");
    tonalExportHint->setObjectName("mutedText");
    tonalExportHint->setWordWrap(true);
    exportCardLayout->addWidget(tonalExportHint);
    auto* actionRow = new QHBoxLayout();
    generateButton_ = new QPushButton("СОЗДАТЬ НАБОР СЭМПЛОВ");
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
    studioLayout->addWidget(makeCard("06  /  ЭКСПОРТ И РЕЗУЛЬТАТ", exportCardLayout));

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
        "<h2>VentuSample — локальная звуковая рабочая станция</h2>"
        "<p>Процедурный генератор хип-хоп-сэмплов с воспроизводимыми seed-вариациями."
        " Все вычисления выполняются локально, WAV экспортируется в PCM16.</p>"
        "<h3>Профили</h3><p>Boom Bap · East Coast · Harlem Dip-Trap · G-Unit · Modern Trap · Atlanta Trap · UK Drill · UK Grime · Memphis · Jazz Rap · G-Funk · Boom Trap · Cripple Trill.</p>"
        "<h3>Что генерируется</h3><p>Полные лупы, мелодии, ударные, басовые партии и наборы одиночных сэмплов."
        " Каждая вариация получает независимый детерминированный seed.</p>"
        "<h3>Инженерные ограничения</h3><p>Офлайн-рендеринг; фиксированные диапазоны параметров; ограничение длительности и числа вариантов;"
        " отсутствие скрытой загрузки библиотек, сетевого телеметрирования и обращения к устройствам записи.</p>"
        "<p>0.1.0 · Windows / Linux / macOS (Qt 6 desktop target)</p>"
        "<p><b>Разработчик:</b> Ivan-VHS</p>");
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
    connect(atmosphereBox_, &QComboBox::currentTextChanged, this, [this](const QString&) { applyTheme(); saveSettings(); });
    connect(swingSlider_, &QSlider::valueChanged, this, [this](int) { saveSettings(); });
}

void MainWindow::showPlugins(const QString& selected) {
    QDialog dialog(this);
    dialog.setWindowTitle("VentuSample — менеджер плагинов");
    dialog.resize(920, 590);
    auto* root = new QVBoxLayout(&dialog);
    auto* intro = new QLabel("19 общих внутренних модулей · встроенный C++ host · версионированный Plugin API");
    intro->setWordWrap(true);
    root->addWidget(intro);

    auto* columns = new QHBoxLayout();
    auto* list = new QListWidget(&dialog);
    auto* details = new QPlainTextEdit(&dialog);
    details->setReadOnly(true);
    details->setMinimumWidth(440);
    columns->addWidget(list, 1);
    columns->addWidget(details, 2);
    root->addLayout(columns, 1);

    auto* actions = new QHBoxLayout();
    auto* discover = new QPushButton("Обнаружить расширения…", &dialog);
    auto* toggle = new QPushButton("Включить / отключить", &dialog);
    auto* run = new QPushButton("Запустить плагин", &dialog);
    auto* close = new QPushButton("Закрыть", &dialog);
    actions->addWidget(discover);
    actions->addWidget(toggle);
    actions->addWidget(run);
    actions->addStretch();
    actions->addWidget(close);
    root->addLayout(actions);

    const auto refresh = [this, list, details, selected] {
        const QString current = list->currentItem()
            ? list->currentItem()->data(Qt::UserRole).toString() : selected;
        list->clear();
        int selectedRow = 0;
        int row = 0;
        for (const auto& plugin : pluginManager_.plugins()) {
            const bool enabled = pluginManager_.isEnabled(plugin.id);
            auto* item = new QListWidgetItem(
                QString::fromStdString(plugin.name) + (enabled ? "   [ВКЛ]" : "   [ВЫКЛ]"), list);
            item->setData(Qt::UserRole, QString::fromStdString(plugin.id));
            if (QString::fromStdString(plugin.id) == current) selectedRow = row;
            ++row;
        }
        if (list->count() > 0) list->setCurrentRow(selectedRow);
        (void)details;
    };
    const auto updateDetails = [this, list, details, toggle, run] {
        const auto* item = list->currentItem();
        if (!item) {
            details->clear();
            toggle->setEnabled(false);
            run->setEnabled(false);
            return;
        }
        const auto id = item->data(Qt::UserRole).toString().toStdString();
        const auto* plugin = pluginManager_.find(id);
        if (!plugin) return;
        const auto statuses = pluginManager_.statuses();
        const auto status = std::find_if(statuses.begin(), statuses.end(),
            [&id](const ventu::PluginStatus& value) { return value.descriptor.id == id; });
        const bool hasHandler = status != statuses.end() && status->handlerAttached;
        const bool enabled = pluginManager_.isEnabled(id);
        QString text = QString("NAME: %1\nID: %2\nVERSION: %3\nAPI: %4\nDOMAIN: %5\nSOURCE: %6\nOPERATION: %7\n\n%8\n\nCAPABILITIES:\n")
            .arg(QString::fromStdString(plugin->name),
                 QString::fromStdString(plugin->id),
                 QString::fromStdString(plugin->version),
                 QString::number(plugin->apiVersion),
                 QString::fromStdString(plugin->domain),
                 QString::fromStdString(plugin->source),
                 QString::fromStdString(plugin->operation),
                 QString::fromStdString(plugin->description));
        for (const auto& capability : plugin->capabilities)
            text += " • " + QString::fromStdString(capability) + "\n";
        text += QString("\nSTATUS: %1\nHANDLER: %2\nMANIFEST: %3")
            .arg(enabled ? "ENABLED" : "DISABLED",
                 hasHandler ? "ATTACHED" : "NOT ATTACHED",
                 QString::fromStdString(plugin->manifestPath));
        if (status != statuses.end() && !status->lastError.empty())
            text += "\nLAST ERROR: " + QString::fromStdString(status->lastError);
        details->setPlainText(text);
        toggle->setEnabled(hasHandler || enabled);
        toggle->setText(enabled ? "Отключить" : "Включить");
        run->setEnabled(enabled && hasHandler);
    };

    refresh();
    connect(discover, &QPushButton::clicked, &dialog, [this, &dialog, details, refresh, updateDetails] {
        const QString directory = QFileDialog::getExistingDirectory(
            &dialog, "Каталог расширений", QCoreApplication::applicationDirPath());
        if (directory.isEmpty()) return;
        std::vector<std::string> diagnostics;
        const auto count = pluginManager_.discoverManifests(pathFromQString(directory), &diagnostics);
        QString report = QString("Обнаружено новых манифестов: %1").arg(static_cast<qulonglong>(count));
        for (const auto& item : diagnostics)
            report += "\n• " + QString::fromStdString(item);
        details->setPlainText(report);
        refresh();
        updateDetails();
    });
    connect(list, &QListWidget::currentRowChanged, &dialog, [updateDetails](int) { updateDetails(); });
    connect(toggle, &QPushButton::clicked, &dialog, [this, list, refresh, updateDetails] {
        const auto* item = list->currentItem();
        if (!item) return;
        const auto id = item->data(Qt::UserRole).toString().toStdString();
        pluginManager_.setEnabled(id, !pluginManager_.isEnabled(id));
        saveSettings();
        refresh();
        updateDetails();
    });
    connect(run, &QPushButton::clicked, &dialog, [this, list, details, updateDetails] {
        const auto* item = list->currentItem();
        if (!item) return;
        const auto id = item->data(Qt::UserRole).toString().toStdString();
        ventu::PluginContext context;
        context.parameters = parameters();
        context.workspaceRoot = pathFromQString(QCoreApplication::applicationDirPath());
        context.outputDirectory = pathFromQString(outputPath_->text());
        const auto result = pluginManager_.execute(id, context);
        QString report = QString::fromStdString(result.summary);
        for (const auto& detail : result.details)
            report += "\n• " + QString::fromStdString(detail);
        details->setPlainText(report);
        appendLog(QString("PLUGIN %1  %2").arg(QString::fromStdString(id),
                                               result.success ? "OK" : "FAILED"));
    });
    connect(close, &QPushButton::clicked, &dialog, &QDialog::accept);
    updateDetails();
    dialog.exec();
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
    stochasticCheck_->setChecked(settings.value("stochastic_enabled", false).toBool());
    stochasticIntensitySlider_->setValue(settings.value("stochastic_intensity", 45).toInt());
    stochasticTriggerBox_->setCurrentIndex(std::clamp(settings.value("stochastic_trigger", 0).toInt(), 0, 2));
    const auto poolMask = static_cast<unsigned int>(settings.value("stochastic_pool_mask", 255).toUInt());
    for (std::size_t effect = 0; effect < stochasticPoolChecks_.size(); ++effect)
        stochasticPoolChecks_[effect]->setChecked((poolMask & (1u << effect)) != 0u);
    stochasticTopologyBox_->setCurrentIndex(std::clamp(settings.value("stochastic_topology", 0).toInt(), 0, 2));
    const QString savedAtmosphere = settings.value("atmosphere", settings.value("dark", false).toBool() ? "Ночь" : "Рассвет").toString();
    setCombo(atmosphereBox_, savedAtmosphere);
    outputPath_->setText(settings.value("output", QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/VentuSample").toString());
    sourceSamplePath_->setText(settings.value("source_sample", "").toString());
    sourceMixSlider_->setValue(settings.value("source_mix", 65).toInt());
    sourcePitchSpin_->setValue(settings.value("source_pitch", 0).toInt());
    settings.endGroup();
    settings.beginGroup("ui_motion");
    motionPreferences_.reduceMotion = settings.value("reduce_motion", false).toBool();
    motionPreferences_.ecoVisuals = settings.value("eco_visuals", false).toBool();
    motionPreferences_.syncToBpm = settings.value("sync_to_bpm", false).toBool();
    settings.endGroup();
    settings.beginGroup("plugins");
    for (const auto& plugin : pluginManager_.plugins()) {
        if (settings.contains(QString::fromStdString(plugin.id)))
            pluginManager_.setEnabled(plugin.id, settings.value(QString::fromStdString(plugin.id)).toBool());
    }
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
    settings.setValue("stochastic_enabled", stochasticCheck_->isChecked());
    settings.setValue("stochastic_intensity", stochasticIntensitySlider_->value());
    settings.setValue("stochastic_trigger", stochasticTriggerBox_->currentIndex());
    unsigned int poolMask = 0u;
    for (std::size_t effect = 0; effect < stochasticPoolChecks_.size(); ++effect)
        if (stochasticPoolChecks_[effect]->isChecked()) poolMask |= (1u << effect);
    settings.setValue("stochastic_pool_mask", static_cast<uint>(poolMask));
    settings.setValue("stochastic_topology", stochasticTopologyBox_->currentIndex());
    settings.setValue("atmosphere", atmosphereBox_->currentText());
    settings.setValue("dark", atmosphereBox_->currentText() == "Ночь");
    settings.setValue("output", outputPath_->text());
    settings.setValue("source_sample", sourceSamplePath_->text());
    settings.setValue("source_mix", sourceMixSlider_->value());
    settings.setValue("source_pitch", sourcePitchSpin_->value());
    settings.endGroup();
    settings.beginGroup("plugins");
    for (const auto& plugin : pluginManager_.plugins())
        settings.setValue(QString::fromStdString(plugin.id), pluginManager_.isEnabled(plugin.id));
    settings.endGroup();
    settings.beginGroup("ui_motion");
    settings.setValue("reduce_motion", motionPreferences_.reduceMotion);
    settings.setValue("eco_visuals", motionPreferences_.ecoVisuals);
    settings.setValue("sync_to_bpm", motionPreferences_.syncToBpm);
    settings.endGroup();
    settings.sync();
}

void MainWindow::showMotionSettings() {
    QDialog dialog(this);
    dialog.setWindowTitle("Настройки движения интерфейса");
    dialog.setMinimumWidth(440);
    auto* layout = new QVBoxLayout(&dialog);
    auto* reduce = new QCheckBox("Reduce Motion — убрать пространственное движение");
    auto* eco = new QCheckBox("Eco / Low-CPU Visuals — отключить необязательные эффекты");
    auto* sync = new QCheckBox("Sync to Transport / BPM — ритмический индикатор");
    reduce->setChecked(motionPreferences_.reduceMotion);
    eco->setChecked(motionPreferences_.ecoVisuals);
    sync->setChecked(motionPreferences_.syncToBpm);
    auto* note = new QLabel("BPM-синхронизация использует темп текущего проекта VentuSample. Внешний транспорт DAW пока не подключён. Reduce Motion и Eco отключают beat-индикатор и ограничивают переходы 80 мс.");
    note->setWordWrap(true);
    layout->addWidget(reduce);
    layout->addWidget(eco);
    layout->addWidget(sync);
    layout->addWidget(note);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return;
    motionPreferences_.reduceMotion = reduce->isChecked();
    motionPreferences_.ecoVisuals = eco->isChecked();
    motionPreferences_.syncToBpm = sync->isChecked();
    saveSettings();
    updateBeatSync();
}

void MainWindow::updateBeatSync() {
    if (!beatIndicator_ || !beatTimer_ || !bpmSpin_) return;
    const bool enabled = motionPreferences_.syncToBpm
        && !motionPreferences_.reduceMotion
        && !motionPreferences_.ecoVisuals;
    beatIndicator_->setVisible(enabled);
    if (!enabled) {
        beatTimer_->stop();
        return;
    }
    const int beatMs = 60000 / std::max(1, bpmSpin_->value());
    beatTimer_->setInterval(std::clamp(beatMs, 250, 1000));
    if (!beatTimer_->isActive()) beatTimer_->start();
}

void MainWindow::applyTheme() {
    const QString atmosphere = atmosphereBox_ ? atmosphereBox_->currentText() : "Рассвет";
    const bool night = atmosphere == "Ночь";
    const bool sunset = atmosphere == "Закат";
    const QString page = night ? "#081522" : sunset ? "#F5EFE8" : "#F3F6FA";
    const QString surface = night ? "#10263B" : sunset ? "#FFF9F2" : "#FFFFFF";
    const QString ink = night ? "#EDF4FA" : sunset ? "#322B2A" : "#10243A";
    const QString secondary = night ? "#C4D5E4" : sunset ? "#68564F" : "#365A83";
    const QString border = night ? "#31536B" : sunset ? "#D8C5B5" : "#CBD6E4";
    const QString accent = "#0039A6";
    const QString signal = night ? "#FF7D78" : "#B51A10";
    const QString signalBright = night ? "#FFA39B" : "#D52B1E";
    const QString selection = night ? "#174C7D" : "#0039A6";
    const QString hoverAccent = night ? "#9ACBFF" : "#0A1E46";

    const QString style = QStringLiteral(R"QSS(
        QWidget {
            background: %1;
            color: %2;
            font-family: "Segoe UI", "Arial", "DejaVu Sans";
            font-size: 10pt;
        }
        QMainWindow { background: %1; }
        QGroupBox {
            background: %3;
            border: 1px solid %4;
            border-left: 3px solid %7;
            border-radius: 3px;
            margin-top: 12px;
            padding: 14px 10px 10px;
            font-weight: 650;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 5px;
            color: %5;
            font-family: "Georgia", "Cambria", "Times New Roman";
            font-size: 10pt;
            font-weight: 700;
        }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QListWidget, QPlainTextEdit {
            min-height: 36px;
            background: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 3px;
            padding: 7px;
            selection-background-color: %8;
            selection-color: #FFFFFF;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus,
        QListWidget:focus, QPlainTextEdit:focus, QPushButton:focus {
            border: 2px solid %9;
        }
        QPushButton {
            background: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 3px;
            padding: 9px 14px;
            min-height: 24px;
        }
        QPushButton:hover { border-color: %9; }
        QPushButton:disabled {
            color: %5;
            background: %1;
            border: 1px solid %4;
        }
        QPushButton#primaryButton {
            background: %6;
            color: #FFFFFF;
            border: 1px solid %6;
            font-weight: 750;
        }
        QPushButton#primaryButton:hover { background: %10; }
        QSlider::groove:horizontal {
            height: 4px;
            background: %4;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal { background: %6; border-radius: 2px; }
        QSlider::handle:horizontal {
            background: #FFFFFF;
            border: 2px solid %6;
            width: 15px;
            margin: -6px 0;
            border-radius: 3px;
        }
        QProgressBar {
            background: %1;
            color: %2;
            border: 1px solid %4;
            border-radius: 3px;
            min-height: 10px;
            text-align: center;
        }
        QProgressBar::chunk { background: %6; border-radius: 2px; }
        QLabel#brandTitle {
            color: %5;
            font-family: "Georgia", "Cambria", "Times New Roman";
            font-size: 22px;
            font-weight: 800;
        }
        QLabel#brandSubtitle, QLabel#fieldLabel { color: %5; font-size: 10px; font-weight: 700; }
        QLabel#heroTitle {
            color: %5;
            font-family: "Georgia", "Cambria", "Times New Roman";
            font-size: 20px;
            font-weight: 750;
        }
        QLabel#heroSubtitle, QLabel#mutedText { color: %5; }
        QLabel#statusPill {
            color: #FFFFFF;
            background: %6;
            border: 1px solid %6;
            border-radius: 3px;
            padding: 8px 12px;
            font-size: 10px;
            font-weight: 800;
        }
        QTabBar::tab {
            background: %3;
            color: %5;
            border: 1px solid %4;
            padding: 10px 16px;
            margin-right: 4px;
            border-top-left-radius: 3px;
            border-top-right-radius: 3px;
        }
        QTabBar::tab:selected {
            color: %5;
            background: %1;
            border-bottom: 3px solid %7;
        }
        QCheckBox { spacing: 8px; }
        QCheckBox::indicator { width: 17px; height: 17px; }
        QCheckBox::indicator:checked {
            background: %6;
            border: 2px solid %6;
        }
        QListWidget::item:selected { background: %8; color: #FFFFFF; }
    )QSS")
        .arg(page, ink, surface, border, secondary, accent, signal, selection, signalBright, hoverAccent);
    setStyleSheet(style);
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
    p.stochasticEnabled = stochasticCheck_->isChecked();
    p.stochasticIntensityPercent = stochasticIntensitySlider_->value();
    p.stochasticTriggerMode = stochasticTriggerBox_->currentIndex();
    p.stochasticTopology = stochasticTopologyBox_->currentIndex();
    p.stochasticPluginMask = 0u;
    for (std::size_t effect = 0; effect < stochasticPoolChecks_.size(); ++effect)
        if (stochasticPoolChecks_[effect]->isChecked()) p.stochasticPluginMask |= (1u << effect);
    return p;
}

void MainWindow::setBusy(bool busy) {
    generateButton_->setEnabled(!busy);
    generateButton_->setText(busy ? "СОЗДАНИЕ НАБОРА СЭМПЛОВ…" : "СОЗДАТЬ SAMPLE PACK");
    progress_->setRange(0, busy ? 0 : 100);
    if (!busy) progress_->setValue(100);
    statusLabel_->setText(busy ? "Рендеринг выполняется в рабочем потоке. Интерфейс остаётся отзывчивым." : statusLabel_->text());
    if (busy) ventu::ui_motion::pulse(statusLabel_, motionPreferences_, 70);
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
    const auto reply = QMessageBox::question(this, "Создать набор сэмплов",
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
    appendLog(QString("KEY  %1 · scale: %2")
        .arg(keyBox_->currentText(), q(ventu::toString(p.scale))));

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
            const auto cadence = result.cadence;
            QMetaObject::invokeMethod(this, [this, files, error, p, cadence] {
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
            double cadenceTotal = 0.0;
            int cadenceCount = 0;
            for (const auto& report : cadence) {
                if (report.valid && report.analyzed) {
                    cadenceTotal += report.gridAccuracyPercent;
                    ++cadenceCount;
                }
            }
            const QString cadenceLabel = cadenceCount > 0
                ? QString("grid %1%").arg(cadenceTotal / cadenceCount, 0, 'f', 1)
                : QString("grid n/a");
            summaryLabel_->setText(QString("Создано файлов: %1 WAV + manifest · %2 BPM · %3 · %4 / %5 · seed %6 · %7 · предел пика −0,3 dBFS")
                .arg(files.size()).arg(p.bpm).arg(q(ventu::toString(p.style)))
                .arg(keyBox_->currentText(), q(ventu::toString(p.scale))).arg(p.seed).arg(cadenceLabel));
            appendLog(QString("QMR CADENCE  %1 analyzed loop exports · %2")
                .arg(cadenceCount).arg(cadenceLabel));
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