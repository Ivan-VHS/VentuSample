#pragma once

#include "generator.hpp"
#include "plugin_manager.hpp"
#include "ui_motion.hpp"

#include <QMainWindow>

#include <array>
#include <QSettings>
#include <QString>
#include <thread>
#include <vector>

class QCloseEvent;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QSlider;
class QPlainTextEdit;
class QTabWidget;
class QTimer;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void showPlugins(const QString& selected = {});
    void loadSettings();
    void saveSettings();
    void applyTheme();
    void showMotionSettings();
    void updateBeatSync();
    ventu::Parameters parameters() const;
    void startGeneration();
    void setBusy(bool busy);
    void appendLog(const QString& message);
    void addChoice(QComboBox* combo, const QStringList& labels, int selected = 0);
    QSlider* addSliderRow(class QGridLayout* grid, int row, const QString& label,
                          int min, int max, int value, QLabel** valueLabel);

    QComboBox* styleBox_{};
    QComboBox* targetBox_{};
    QComboBox* keyBox_{};
    QComboBox* scaleBox_{};
    QComboBox* samplerBox_{};
    QComboBox* timbreBox_{};
    QComboBox* rateBox_{};
    QSpinBox* bpmSpin_{};
    QSpinBox* barsSpin_{};
    QSpinBox* variationsSpin_{};
    QSpinBox* seedSpin_{};
    QSpinBox* snareOffsetSpin_{};
    QDoubleSpinBox* ratioSpin_{};
    QSlider* swingSlider_{};
    QSlider* densitySlider_{};
    QSlider* gritSlider_{};
    QSlider* saturationSlider_{};
    QSlider* stereoSlider_{};
    QSlider* hpSlider_{};
    QSlider* lpSlider_{};
    QLabel* swingValue_{};
    QLabel* densityValue_{};
    QLabel* gritValue_{};
    QLabel* saturationValue_{};
    QLabel* stereoValue_{};
    QLabel* hpValue_{};
    QLabel* lpValue_{};
    QCheckBox* tripletCheck_{};
    QCheckBox* humanizeCheck_{};
    QCheckBox* dustCheck_{};
    QCheckBox* stochasticCheck_{};
    QSlider* stochasticIntensitySlider_{};
    QLabel* stochasticIntensityValue_{};
    QComboBox* stochasticTriggerBox_{};
    QComboBox* stochasticTopologyBox_{};
    std::array<QCheckBox*, 8> stochasticPoolChecks_{};
    QCheckBox* darkThemeCheck_{};
    QLineEdit* outputPath_{};
    QLineEdit* sourceSamplePath_{};
    QSlider* sourceMixSlider_{};
    QSpinBox* sourcePitchSpin_{};
    QLabel* sourceMixValue_{};
    QPushButton* generateButton_{};
    QPushButton* openFolderButton_{};
    QProgressBar* progress_{};
    QLabel* statusLabel_{};
    QLabel* summaryLabel_{};
    QLabel* beatIndicator_{};
    QTabWidget* tabs_{};
    QTimer* beatTimer_{};
    ventu::ui_motion::Preferences motionPreferences_{};
    QListWidget* filesList_{};
    QPlainTextEdit* log_{};
    ventu::PluginManager pluginManager_;
    std::thread worker_;
    QString lastOutputDirectory_;
};
