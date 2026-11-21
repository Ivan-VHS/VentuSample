# Bounded resource allocation: 500 UAH

This is a portable, illustrative personal-budgeting reference. It is not a runtime feature of the host project, a live price feed or financial advice. The same reference model is provided in each repository for consistency.

## English

### Resource model

Treat 500 UAH as a finite local cap. Amounts are whole UAH values. Each allocation must be a non-negative integer; the sum must not exceed the cap; the remainder is cap minus the sum. Reject overspending rather than silently borrowing from a future period.

The JSON Schema validates structure and per-field bounds. The consumer must also calculate the cross-field sum: standard JSON Schema does not express this arithmetic rule.

### Allocation domains

| Category | Purpose | Examples |
|---|---|---|
| groceries_core | basic food | grains, pasta, bread, eggs, seasonal vegetables, fruit, chicken |
| transit_pool | mobility | public transport balance, e-ticket, capped taxi reserve |
| caffeine_ping | optional small comfort | coffee, tea or a quick snack |
| household_health | household essentials and first aid | detergent, soap, toothpaste, batteries, plasters, antiseptic |
| knowledge_or_leisure | learning or bounded recreation | book, course material, cinema or a low-cost social activity |
| spare_buffer | exceptions | water, unexpected fare or price difference |

A two-bucket grocery split can protect the core basket: approximately 250–300 UAH for staples and vegetables, and 200–250 UAH for protein and dairy. Adjust it to actual prices, package sizes and home inventory.

### Scenario presets

All amounts are illustrative presets, not current market quotations.

| Preset | Groceries | Transit | Caffeine | Household | Learning / leisure | Reserve | Total |
|---|---:|---:|---:|---:|---:|---:|---:|
| balanced | 280 | 120 | 50 | 0 | 0 | 50 | 500 |
| survival_batch | 370 | 50 | 0 | 30 | 0 | 50 | 500 |
| transit_buffer | 260 | 160 | 0 | 30 | 0 | 50 | 500 |
| context_reset | 250 | 80 | 70 | 50 | 0 | 50 | 500 |
| tech_debt_cleanup | 260 | 80 | 0 | 110 | 0 | 50 | 500 |
| knowledge_upgrade | 260 | 80 | 0 | 30 | 80 | 50 | 500 |

### Operating rules

1. Choose a mode first: needs, reward or balanced. Keep rewards within an explicit cap.
2. Compare unit prices (per kg, litre or item); a larger pack is only cheaper if it will be used before expiry.
3. Protect a 5–10% reserve when the next income date or travel costs are uncertain.
4. Use an e-ticket or transport card only when the actual tariff and expected trips make it cheaper.
5. Log actual purchases on the day they happen; keep planned amounts separate from actual spend.
6. Recalculate when prices change. Local fares and retail prices vary by city, seller, date and payment method; this reference guarantees none of them.

For health-related purchases, limit this category to ordinary first-aid supplies and medicines already prescribed or approved by a qualified clinician. Do not self-prescribe antibiotics or use this template to choose treatment.

### Balanced weekly example

| Category | UAH |
|---|---:|
| groceries_core | 280 |
| transit_pool | 120 |
| caffeine_ping | 50 |
| household_health | 0 |
| knowledge_or_leisure | 0 |
| spare_buffer | 50 |
| **Checksum** | **500** |

Decision routing: empty fridge → groceries first; high mobility → transit first; low morale → use only the pre-approved reward amount; depleted essentials → household ticket first; no immediate need → keep the full amount as a cold reserve.

The offline simulator lets a user change category amounts, select a preset and see the remainder or overrun immediately. It does not store, transmit or verify purchases.

---

## Русский

### Модель ресурса

Рассматривайте 500 грн как ограниченный локальный буфер. Суммы задаются целыми гривнами; каждая категория неотрицательна; общая сумма не превышает лимит; остаток равен лимиту минус сумма распределений. Перерасход нужно отклонять, а не маскировать заимствованием из будущего периода.

JSON Schema проверяет структуру и пределы полей. Дополнительно приложение обязано суммировать категории: стандартная JSON Schema не выражает это арифметическое условие.

### Категории

| Категория | Назначение |
|---|---|
| groceries_core | базовое питание: крупы, макароны, хлеб, яйца, овощи, фрукты, курица |
| transit_pool | транспортная карта, e-ticket или ограниченный резерв на такси |
| caffeine_ping | кофе, чай или небольшой перекус |
| household_health | бытовые расходники и базовая первая помощь |
| knowledge_or_leisure | книга, учебный материал или дозированный досуг |
| spare_buffer | вода, внеплановый проезд или расхождение цен |

Для продуктовой корзины можно использовать два сегмента: 250–300 грн на базовые продукты и овощи, 200–250 грн на белковые и молочные продукты. Корректируйте доли по фактическим ценам, фасовкам и запасам дома.

### Сценарные пресеты

Это примерные распределения, а не актуальные котировки магазинов и перевозчиков.

| Пресет | Продукты | Транспорт | Кофе | Быт | Знания / досуг | Резерв | Итого |
|---|---:|---:|---:|---:|---:|---:|---:|
| balanced | 280 | 120 | 50 | 0 | 0 | 50 | 500 |
| survival_batch | 370 | 50 | 0 | 30 | 0 | 50 | 500 |
| transit_buffer | 260 | 160 | 0 | 30 | 0 | 50 | 500 |
| context_reset | 250 | 80 | 70 | 50 | 0 | 50 | 500 |
| tech_debt_cleanup | 260 | 80 | 0 | 110 | 0 | 50 | 500 |
| knowledge_upgrade | 260 | 80 | 0 | 30 | 80 | 50 | 500 |

### Правила эксплуатации

1. Сначала выберите режим: необходимость, награда или сбалансированный. Для наград задайте отдельный лимит.
2. Сравнивайте цену за килограмм, литр или единицу. Большая упаковка выгодна только при использовании до истечения срока годности.
3. При неопределённости до следующего поступления денег или по транспортным расходам сохраняйте 5–10% резерва.
4. Транспортную карту используйте, только если фактический тариф и число поездок дают экономию.
5. Записывайте реальные расходы в день покупки, отдельно от плана.
6. Пересчитывайте суммы при изменении цен. Тарифы зависят от города, продавца, даты и оплаты; здесь нет гарантированных цен.

Аптечная часть предназначена для базовой первой помощи и препаратов, уже назначенных или одобренных медицинским специалистом. Не назначайте себе антибиотики самостоятельно и не используйте шаблон для выбора лечения.

### Сбалансированный пример на неделю

280 грн — продукты, 120 грн — транспорт, 50 грн — кофе/перекус, 50 грн — резерв. Контрольная сумма: **280 + 120 + 50 + 50 = 500 грн**.

Пустой холодильник → продуктовая корзина; много перемещений → транспорт; низкая мотивация → только ограниченная сумма на награду; бытовые нехватки → необходимые расходники; срочных задач нет → холодный резерв.

Симулятор работает локально, пересчитывает остаток и превышение в реальном времени и не хранит историю покупок.

---

## Українська

### Модель ресурсу

500 грн — обмежений локальний бюджетний буфер. Усі суми задаються цілими гривнями; кожна категорія невід’ємна; сума розподілу не перевищує ліміт; залишок дорівнює ліміту мінус усі категорії. Перевищення потрібно явно відхиляти.

JSON Schema перевіряє структуру та межі полів. Загальну суму категорій потрібно додатково перевіряти програмно.

### Категорії

| Категорія | Призначення |
|---|---|
| groceries_core | базові продукти: крупи, макарони, хліб, яйця, овочі, фрукти, курятина |
| transit_pool | громадський транспорт, e-ticket або обмежений резерв на таксі |
| caffeine_ping | необов’язкова кава, чай чи перекус |
| household_health | побутові засоби та базова перша допомога |
| knowledge_or_leisure | книга, навчальні матеріали або дозвілля з лімітом |
| spare_buffer | вода, непередбачена поїздка чи різниця в ціні |

Для продуктів можна розділити суму на два кошики: 250–300 грн на базові продукти й овочі та 200–250 грн на білкові й молочні продукти. Коригуйте частки за реальними цінами та запасами вдома.

### Сценарії

| Пресет | Продукти | Транспорт | Кава | Побут | Навчання / дозвілля | Резерв | Разом |
|---|---:|---:|---:|---:|---:|---:|---:|
| balanced | 280 | 120 | 50 | 0 | 0 | 50 | 500 |
| survival_batch | 370 | 50 | 0 | 30 | 0 | 50 | 500 |
| transit_buffer | 260 | 160 | 0 | 30 | 0 | 50 | 500 |
| context_reset | 250 | 80 | 70 | 50 | 0 | 50 | 500 |
| tech_debt_cleanup | 260 | 80 | 0 | 110 | 0 | 50 | 500 |
| knowledge_upgrade | 260 | 80 | 0 | 30 | 80 | 50 | 500 |

Це демонстраційні розподіли, а не поточні ринкові ціни чи тарифи.

### Правила

- Спочатку оберіть режим: потреби, винагорода або збалансований. Для винагород встановіть окремий ліміт.
- Порівнюйте ціну за кілограм, літр або одиницю; велика упаковка вигідна лише за використання до завершення строку придатності.
- Зберігайте 5–10% резерву, якщо дата наступного надходження або транспортні витрати невизначені.
- Перевіряйте фактичні місцеві тарифи та ціни перед витратою.
- Записуйте фактичні покупки окремо від плану.
- Аптечна категорія призначена для базової першої допомоги та препаратів, призначених або схвалених медичним фахівцем. Не призначайте собі антибіотики.

Контрольний приклад: 280 грн — продукти, 120 грн — транспорт, 50 грн — кава/перекус, 50 грн — резерв. **280 + 120 + 50 + 50 = 500 грн**.

Порожній холодильник — пріоритет продуктам; висока мобільність — транспорту; побутові нестачі — необхідним витратникам; немає термінових потреб — збереженню всього буфера. Симулятор працює локально та не передає дані.

---

## 简体中文

### 有界资源模型

将 500 UAH 视为有限的本地预算缓冲区。金额使用整数格里夫纳；每个类别金额必须非负；分配总额不得超过上限；剩余金额等于预算上限减去所有分配。超预算必须明确拒绝。

JSON Schema 检查数据结构和单字段范围；应用仍须自行计算所有类别的总和。

### 分配类别

| 类别 | 用途 |
|---|---|
| groceries_core | 基础食品：谷物、意面、面包、鸡蛋、蔬果、鸡肉 |
| transit_pool | 公共交通、电子票或有限出租车备用金 |
| caffeine_ping | 可选咖啡、茶或小吃 |
| household_health | 家居消耗品和基础急救用品 |
| knowledge_or_leisure | 书籍、学习资料或有额度的休闲活动 |
| spare_buffer | 饮水、临时交通或价格差额 |

食品预算可分为两个部分：250–300 UAH 用于主食和蔬菜，200–250 UAH 用于蛋白质和乳制品。请依据实际价格、包装规格和家中库存调整。

### 示例预设

| 预设 | 食品 | 交通 | 咖啡 | 家居 | 学习 / 休闲 | 储备 | 合计 |
|---|---:|---:|---:|---:|---:|---:|---:|
| balanced | 280 | 120 | 50 | 0 | 0 | 50 | 500 |
| survival_batch | 370 | 50 | 0 | 30 | 0 | 50 | 500 |
| transit_buffer | 260 | 160 | 0 | 30 | 0 | 50 | 500 |
| context_reset | 250 | 80 | 70 | 50 | 0 | 50 | 500 |
| tech_debt_cleanup | 260 | 80 | 0 | 110 | 0 | 50 | 500 |
| knowledge_upgrade | 260 | 80 | 0 | 30 | 80 | 50 | 500 |

这些是分配示例，不代表当前零售价格或交通票价。

### 运行规则

1. 先选择 needs、reward 或 balanced 模式，并为奖励消费设置单独上限。
2. 比较每公斤、每升或每件的单价；只有能在保质期内用完时，大包装才可能更划算。
3. 当收入日期或交通费用不确定时，保留 5–10% 备用金。
4. 购买前核实当地实时价格和交通票价。
5. 当日记录实际支出，并与计划金额分开。
6. 医疗相关支出仅用于基础急救用品和专业医务人员已开具或批准的药物。不要自行使用抗生素。

平衡示例：食品 280 UAH、交通 120 UAH、咖啡/小吃 50 UAH、备用金 50 UAH。校验总额：**280 + 120 + 50 + 50 = 500 UAH**。

冰箱空了，优先购买食品；出行较多，优先交通；家居用品短缺，优先必要消耗品；没有紧急需求时，保留完整备用金。交互模拟器在本地运行，不存储或传输购买记录。
