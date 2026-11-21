#include "budget_planner_dialog.hpp"

#include "budget_planner.hpp"

#include <QComboBox>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSaveFile>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>
#include <array>

namespace ventu {
void showBudgetPlanner(QWidget* parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle(QStringLiteral("VentuSample — планировщик бюджета 500 грн"));
    dialog.setMinimumSize(640, 620);
    dialog.resize(760, 700);

    auto* root = new QVBoxLayout(&dialog);
    auto* title = new QLabel(QStringLiteral("МИКРОБЮДЖЕТ · 500 UAH"));
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 4);
    title->setFont(titleFont);
    root->addWidget(title);

    auto* intro = new QLabel(QStringLiteral(
        "Распределите фиксированный лимит между повседневными категориями. "
        "Цены ниже — редактируемые ориентиры из предоставленной статьи, не live-котировки."));
    intro->setWordWrap(true);
    root->addWidget(intro);

    const auto scenarios = budget::scenarioCatalog();
    auto* scenarioBox = new QComboBox();
    for (const auto& scenario : scenarios) {
        scenarioBox->addItem(QString::fromUtf8(scenario.label.data(),
                           static_cast<qsizetype>(scenario.label.size())),
                           QString::fromUtf8(scenario.id.data(),
                           static_cast<qsizetype>(scenario.id.size())));
    }
    auto* limitSpin = new QSpinBox();
    limitSpin->setRange(0, budget::kMaximumUah);
    limitSpin->setSingleStep(50);
    limitSpin->setValue(budget::kDefaultLimitUah);
    limitSpin->setSuffix(QStringLiteral(" грн"));
    limitSpin->setAccessibleName(QStringLiteral("Общий лимит бюджета"));

    auto* topForm = new QFormLayout();
    topForm->addRow(QStringLiteral("Сценарий"), scenarioBox);
    topForm->addRow(QStringLiteral("Общий лимит"), limitSpin);
    root->addLayout(topForm);

    auto* allocationGroup = new QGroupBox(QStringLiteral("РАСПРЕДЕЛЕНИЕ ПО КАТЕГОРИЯМ"));
    auto* allocationGrid = new QGridLayout(allocationGroup);
    const auto labels = budget::categoryNames();
    std::array<QSpinBox*, budget::kCategoryCount> amountSpins{};
    const auto defaults = budget::defaultPlan().allocations;
    for (std::size_t index = 0; index < budget::kCategoryCount; ++index) {
        auto* label = new QLabel(QString::fromUtf8(labels[index].data(),
                            static_cast<qsizetype>(labels[index].size())));
        auto* spin = new QSpinBox();
        spin->setRange(0, budget::kMaximumUah);
        spin->setSingleStep(10);
        spin->setValue(defaults[index]);
        spin->setSuffix(QStringLiteral(" грн"));
        spin->setAccessibleName(label->text());
        amountSpins[index] = spin;
        allocationGrid->addWidget(label, static_cast<int>(index), 0);
        allocationGrid->addWidget(spin, static_cast<int>(index), 1);
    }
    allocationGrid->setColumnStretch(0, 1);
    root->addWidget(allocationGroup);

    auto* progress = new QProgressBar();
    progress->setRange(0, budget::kDefaultLimitUah);
    progress->setValue(budget::kDefaultLimitUah);
    progress->setTextVisible(true);
    progress->setFormat(QStringLiteral("%v / %m грн"));
    progress->setAccessibleName(QStringLiteral("Использование бюджета"));
    root->addWidget(progress);

    auto* summary = new QLabel();
    summary->setWordWrap(true);
    summary->setTextInteractionFlags(Qt::TextSelectableByMouse);
    root->addWidget(summary);

    auto* references = new QLabel(QStringLiteral(
        "ОРИЕНТИРЫ ИЗ СТАТЬИ\n"
        "Эспрессо — около 46 грн; 10 чашек — 460 грн.\n"
        "Гречка — 75–88 грн/кг · рис — 52–61 · макароны — 40–43.\n"
        "Молоко — 50–54 грн/л · яйца — 40–61 за 10 шт.\n"
        "Днепр (оценки из статьи): автобус 23 грн, трамвай/троллейбус 25, метро 10.\n"
        "Данные не проверены в реальном времени и могут отличаться от фактических цен."));
    references->setWordWrap(true);
    root->addWidget(references);

    const auto collectPlan = [&]() {
        budget::Plan plan;
        plan.limitUah = limitSpin->value();
        for (std::size_t index = 0; index < budget::kCategoryCount; ++index)
            plan.allocations[index] = amountSpins[index]->value();
        return plan;
    };
    const auto refresh = [&]() {
        auto plan = collectPlan();
        std::string error;
        if (!budget::calculate(plan, &error)) {
            summary->setText(QString::fromUtf8(error.c_str()));
            progress->setValue(0);
            return;
        }
        progress->setRange(0, std::max(1, plan.limitUah));
        progress->setValue(std::min(plan.plannedUah, plan.limitUah));
        progress->setFormat(QStringLiteral("%1 / %2 грн")
            .arg(plan.plannedUah).arg(plan.limitUah));
        if (plan.withinBudget) {
            summary->setText(QStringLiteral("Запланировано: %1 грн\nОстаток: %2 грн")
                .arg(plan.plannedUah).arg(plan.remainingUah));
        } else {
            summary->setText(QStringLiteral("Запланировано: %1 грн\nПревышение лимита: %2 грн")
                .arg(plan.plannedUah).arg(-plan.remainingUah));
        }
    };

    QObject::connect(scenarioBox, qOverload<int>(&QComboBox::currentIndexChanged),
        &dialog, [&, scenarioBox](int) {
            const auto id = scenarioBox->currentData().toString().toStdString();
            auto plan = budget::defaultPlan();
            if (budget::applyScenario(plan, id)) {
                for (std::size_t index = 0; index < budget::kCategoryCount; ++index)
                    amountSpins[index]->setValue(plan.allocations[index]);
            }
            refresh();
        });
    QObject::connect(limitSpin, qOverload<int>(&QSpinBox::valueChanged), &dialog, refresh);
    for (auto* spin : amountSpins)
        QObject::connect(spin, qOverload<int>(&QSpinBox::valueChanged), &dialog, refresh);

    auto* actions = new QHBoxLayout();
    auto* exportButton = new QPushButton(QStringLiteral("Экспорт JSON…"));
    auto* closeButton = new QPushButton(QStringLiteral("Закрыть"));
    actions->addWidget(exportButton);
    actions->addStretch();
    actions->addWidget(closeButton);
    root->addLayout(actions);

    QObject::connect(exportButton, &QPushButton::clicked, &dialog, [&]() {
        const auto plan = collectPlan();
        std::string error;
        auto validated = plan;
        if (!budget::calculate(validated, &error)) {
            QMessageBox::warning(&dialog, QStringLiteral("Некорректный бюджет"),
                                 QString::fromUtf8(error.c_str()));
            return;
        }
        QJsonArray rows;
        for (std::size_t index = 0; index < budget::kCategoryCount; ++index) {
            QJsonObject row;
            row.insert(QStringLiteral("category"),
                QString::fromUtf8(labels[index].data(), static_cast<qsizetype>(labels[index].size())));
            row.insert(QStringLiteral("amount_uah"), plan.allocations[index]);
            rows.append(row);
        }
        QJsonObject output{
            {QStringLiteral("schema"), QStringLiteral("ventusample.microbudget.v1")},
            {QStringLiteral("currency"), QStringLiteral("UAH")},
            {QStringLiteral("budget_uah"), plan.limitUah},
            {QStringLiteral("allocations"), rows},
            {QStringLiteral("planned_uah"), validated.plannedUah},
            {QStringLiteral("remaining_uah"), validated.remainingUah},
            {QStringLiteral("over_budget_uah"), std::max(0, -validated.remainingUah)},
            {QStringLiteral("within_budget"), validated.withinBudget},
            {QStringLiteral("price_basis"), QStringLiteral("User-supplied article estimates; not live verified prices.")}
        };
        const QString path = QFileDialog::getSaveFileName(&dialog,
            QStringLiteral("Сохранить план бюджета"), QStringLiteral("budget-500-uah.json"),
            QStringLiteral("JSON (*.json)"));
        if (path.isEmpty()) return;
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly) ||
            file.write(QJsonDocument(output).toJson(QJsonDocument::Indented)) < 0 ||
            !file.commit()) {
            QMessageBox::critical(&dialog, QStringLiteral("Ошибка сохранения"),
                                  QStringLiteral("Не удалось записать JSON-файл."));
            return;
        }
        QMessageBox::information(&dialog, QStringLiteral("План сохранён"),
                                 QStringLiteral("JSON-файл успешно сохранён."));
    });
    QObject::connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    refresh();
    dialog.exec();
}
} // namespace ventu
