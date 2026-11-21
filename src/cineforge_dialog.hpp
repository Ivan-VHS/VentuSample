#pragma once

#include <QDialog>
#include <thread>

class QLineEdit;
class QSpinBox;
class QComboBox;
class QLabel;
class QPushButton;

class CineForgeDialog final : public QDialog {
    Q_OBJECT
public:
    explicit CineForgeDialog(QWidget* parent = nullptr);
    ~CineForgeDialog() override;

private:
    void chooseOutput();
    void generate();

    QLineEdit* prompt_{};
    QLineEdit* output_{};
    QSpinBox* bpm_{};
    QSpinBox* bars_{};
    QSpinBox* packSize_{};
    QSpinBox* seed_{};
    QComboBox* key_{};
    QComboBox* sampleRate_{};
    QComboBox* encoding_{};
    QLabel* status_{};
    QPushButton* generateButton_{};
    std::thread worker_;
};
