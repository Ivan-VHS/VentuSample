#pragma once

#include "stochastic_routing.hpp"

#include <QDialog>
#include <thread>
#include <vector>

class QLineEdit;
class QSpinBox;
class QComboBox;
class QLabel;
class QPushButton;
class QListWidget;

class StochasticDialog final : public QDialog {
    Q_OBJECT
public:
    explicit StochasticDialog(QWidget* parent = nullptr);
    ~StochasticDialog() override;

private:
    void importSource();
    void buildPlans();
    void updatePlanView();
    void toggleSelectedLock();
    void renderSelectedPlan();

    QLineEdit* sourcePath_{};
    QComboBox* mode_{};
    QComboBox* branches_{};
    QComboBox* planSelector_{};
    QSpinBox* seed_{};
    QSpinBox* minPlugins_{};
    QSpinBox* maxPlugins_{};
    QSpinBox* roundRobinCount_{};
    QListWidget* slots_{};
    QLabel* status_{};
    QPushButton* lockButton_{};
    QPushButton* renderButton_{};
    ventu::AudioSuiteBuffer source_;
    std::vector<ventu::StochasticPlan> plans_;
    std::vector<ventu::StochasticSlot> currentSlots_;
    std::thread worker_;
};
