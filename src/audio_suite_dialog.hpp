#pragma once

#include "audio_suite.hpp"

#include <QDialog>
#include <filesystem>
#include <thread>
#include <vector>

class QLabel;
class QListWidget;
class QPushButton;
class QSlider;

class AudioSuiteDialog final : public QDialog {
public:
    explicit AudioSuiteDialog(QWidget* parent = nullptr);
    ~AudioSuiteDialog() override;

private:
    void showLibraryDetails();
    void refreshRack();
    void loadInput();
    void addSelectedPlugin();
    void removeSelectedPlugin();
    void updateSelectedParameters();
    void loadSelectedParameters();
    void exportProcessedRack();

    ventu::AudioPluginParameters parametersFromControls() const;
    void setParameterControls(const ventu::AudioPluginParameters& parameters);
    void setProcessingEnabled(bool enabled);

    QListWidget* library_{};
    QListWidget* rackList_{};
    QLabel* description_{};
    QLabel* sourceLabel_{};
    QLabel* status_{};
    QSlider* amount_{};
    QSlider* wet_{};
    QSlider* rate_{};
    QSlider* tone_{};
    QSlider* width_{};
    QPushButton* importButton_{};
    QPushButton* addButton_{};
    QPushButton* removeButton_{};
    QPushButton* renderButton_{};
    std::filesystem::path sourcePath_;
    ventu::AudioBuffer sourceBuffer_;
    std::vector<ventu::AudioPluginChainItem> rack_;
    std::thread worker_;
};
