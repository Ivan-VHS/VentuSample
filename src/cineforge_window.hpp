#pragma once

#include <QDialog>

class CineforgeWindow final : public QDialog {
    Q_OBJECT
public:
    explicit CineforgeWindow(QWidget* parent = nullptr);
private:
    void generatePack();
};
