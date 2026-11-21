#pragma once

#include <QDialog>

class QCloseEvent;
class QThread;

class CineforgeWindow final : public QDialog {
    Q_OBJECT
public:
    explicit CineforgeWindow(QWidget* parent = nullptr);
private:
    void generatePack();
    void closeEvent(QCloseEvent* event) override;
    QThread* worker_{};
    bool generationPending_{false};
};
