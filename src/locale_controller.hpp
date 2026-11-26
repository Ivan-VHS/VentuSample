#pragma once

#include <QLocale>
#include <QString>
#include <Qt>

namespace ventu {

class LocaleController final {
public:
    static LocaleController fromEnvironment();

    const QString& requestedTag() const noexcept;
    const QLocale& locale() const noexcept;
    Qt::LayoutDirection direction() const noexcept;

private:
    LocaleController(QString requestedTag, QLocale locale, Qt::LayoutDirection direction);

    QString requestedTag_;
    QLocale locale_;
    Qt::LayoutDirection direction_;
};

} // namespace ventu
