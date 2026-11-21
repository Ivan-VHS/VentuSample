# Market development, positioning and commercialization standard

## Purpose and boundary

This standard turns market, commercial, advertising, branding, distribution and customer vocabulary into an evidence-led product practice. It is a planning and governance layer, not a claim that demand, revenue, conversion, market share or profitability has been achieved.

The project-specific source of truth is `market-development.v1.json`, validated against `schemas/market-development.v1.schema.json`. It records intended audiences, product promise, differentiators, go-to-market stages, promotion channels, commercial gates and measurement policy. **Implemented** requires a real repository artifact or existing channel; **planned** is an intention; **not_assessed** means evidence has not been collected.

## Market and customer operating model

1. **Discover the job, not just the demographic.** Describe the user's task, workaround, friction and desired outcome.
2. **Position from verifiable behaviour.** State the category, primary use case, concrete benefit and evidence. Keep technical proof separate from roadmap.
3. **Package a clear offer.** Explain included outputs, formats, prerequisites, limitations, license and first successful workflow.
4. **Promote through relevant channels.** Prefer reproducible demonstrations, release notes, specialist communities and user-requested updates. Advertising must be factual and attributable.
5. **Close the feedback loop.** Convert issues and structured feedback into prioritized product work. Do not infer broad demand from a handful of comments.
6. **Gate commercialization.** Pricing and paid distribution remain unset until demand, source rights, operating cost, support obligations and license compatibility have been reviewed.

## Vocabulary-to-control crosswalk

Repeated vocabulary clusters are consolidated into 20 operational controls; repeated terms do not create duplicate requirements.

| # | Vocabulary cluster | Operational control |
|---:|---|---|
| 1 | Маркетизация · Рыночное · Маркетинг | Define the market, category and user problem before choosing promotional tactics. |
| 2 | Коммерциализация · Коммерческое · Продвижение | Turn verified product value into a clearly scoped offer and release path. |
| 3 | Рыночнизация · Рекламное · Рекламирование | Use audience-relevant, factual promotion with claims tied to evidence. |
| 4 | Продвижение · Продвиженческое · Продвиженчество | Maintain a measurable discovery-to-first-success journey. |
| 5 | Рекламизация · Сбытовое · Рыночное продвижение | Align launch messages, release notes and distribution pages. |
| 6 | Брендизация · Предпринимательское · Коммерциализация | Express a consistent product promise and defensible reason to choose it. |
| 7 | Товаризация · Торговое · Брендинг | Package features, formats, prerequisites, license and limitations as one offer. |
| 8 | Монетизация · Брендинговое · Позиционирование | Keep monetization separate from positioning; do not set prices without evidence. |
| 9 | Коммерциализирование · Клиентское · Медиапродвижение | Publish useful, audience-specific media and technical explanations. |
| 10 | Рыночное освоение · Потребительское · Рекламная деятельность | Validate needs through observed workflows and consent-based feedback. |
| 11 | Предприниматизация · Конъюнктурное · Рыночная деятельность | Review relevant market changes without chasing unsupported trends. |
| 12 | Сбытовая интеграция · Дистрибуционное · Предпринимательство | Make pack retrieval, integrity checks and first use explicit and reproducible. |
| 13 | Позиционирование · Товарное · Сбытовая деятельность | Keep category, product promise and user journey consistent. |
| 14 | Популяризация · Экономическое · Товаропродвижение | Demonstrate value with previews, example packs and reproducible outcomes. |
| 15 | Клиентизация · Финансовое · Клиентопривлечение | Reduce onboarding friction and distinguish users, evaluators and purchasers. |
| 16 | Промоизация · Коммуникационное · Популяризация | Use a consistent editorial voice across documentation, releases and community posts. |
| 17 | Дистрибутизация · Позиционирующее · Монетизация | Measure distribution separately from paid conversion and revenue. |
| 18 | Рыночная адаптация · Коммерциализационное · Коммерческое развитие | Update the offer and roadmap from evidence, not assumptions. |
| 19 | Коммерческое развитие · Рыночно-ориентированное · Рыночное освоение | Prioritize changes by producer value, feasibility, rights risk and maintenance cost. |
| 20 | Маркетинговая интеграция · Промоциональное · Маркетинговая стратегия | Keep messages, release artifacts and project boundaries aligned across the federation. |

## Project-specific positioning

**VentuSample** is an offline, reproducible sample-pack workstation for producers who want controllable hip-hop material and direct WAV exports.

**Primary audiences:** independent beatmakers and hip-hop producers; sample-pack creators and sound designers; music-production educators and technical creators.

**Brand promise:** Give producers repeatable control over the creation and export of original sample-pack material.

**Message pillars:** offline workflow; repeatable generation; audition-ready WAV output; clear provenance and usage limits.

### Proof-led promotion

- Demonstrate seed and parameter repeatability with the same inputs and exported WAV/manifest pair.
- Show real stereo output previews and disclose the generator build, settings and format.
- Explain chop import, slicing and semitone resampling with rights-cleared example material.
- Keep claims about audio quality subjective unless supported by defined listening methodology.
- Do not imply that procedural generation clears copyright in imported samples or third-party recordings.

## Minimum go-to-market sequence

- **Discover:** record the producer segment, job, alternatives and evidence gaps.
- **Validate:** observe real workflows; record feedback with context and date.
- **Explain:** publish a concise value proposition, supported capabilities and non-claims.
- **Distribute:** provide a traceable source/release path, format details, integrity information and license notices.
- **Retain:** maintain format compatibility, changelog and support route.
- **Review:** revisit positioning and priorities when evidence materially changes the picture.

## Measurement and ethical promotion

Use only metrics with an explicit definition and reliable source. Repository views, release downloads, first completed pack, repeat use, pack integrity and paid conversion are different measures. Mark unavailable measures as **not_assessed**.

Do not add hidden telemetry, fingerprinting, undisclosed tracking, fake testimonials or unverified audio-quality comparisons. Any future in-product analytics must be documented, proportionate, privacy-reviewed and user-controlled. Do not repurpose user feedback as a testimonial without permission.

## Commercial release gates

- **Market validation:** observed producer workflows and documented feedback.
- **Pricing:** a reviewed model supported by willingness-to-pay evidence; otherwise keep it unset.
- **Unit economics:** measured generation, curation, hosting, platform, distribution and support costs.
- **Licensing and rights:** manual review of imported samples, loops, recordings, trademarks and redistribution terms.
- **Support:** named ownership, format compatibility policy and maintenance lifecycle.
- **Distribution:** reproducible exports, integrity information, release notes and clear usage rights.

A gate is not passed merely because the repository contains a plan or profile file.
