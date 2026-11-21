# Code Mental Health — Codebase Wellbeing v1

## Purpose

**«Психическое здоровье кода»** is an engineering metaphor for repository sustainability, not a claim that software has human feelings or a clinical mental state. The practical meaning is: can a team see problems early, discuss failures without blame, understand the system, make safe changes, recover from regressions and give maintainers enough support and time to care for the codebase?

This is a deliberate redesign of the earlier wellbeing integration. The previous human-health catalogue, medical framing and medication content are not part of this feature.

## Inspiration and attribution

The communication themes are inspired by the supplied excerpts of [ебаут — «Депресія — тема табу? Ознаки депресії, міфи довкола антидепресантів, як допомогти собі»](https://www.youtube.com/watch?v=brNbnipJ-i8), published 2025-11-23 (duration 1:20:01).

The supplied transcript includes 00:00–00:07:55 and 01:17:16–01:19:53. Most of the middle section is absent. Only broad themes such as destigmatising discussion, asking for support, shared context and a support structure are borrowed as an analogy. The video is about people, not software. The engineering principles and examples below are newly authored; they are not presented as statements made in the video.

## Engineering translation

| Human conversation theme | Repository-care translation |
|---|---|
| Speak openly without stigma | Report bugs, incidents, flaky tests and regressions without blaming the reporter |
| Share experience | Preserve decision records, incident timelines, postmortems and migration notes |
| People and support | Maintain ownership maps, code review paths, pair debugging and escalation routes |
| Physical activity / care routines | Reserve recurring capacity for refactoring, dependency care and test repair |
| Professional support | Ask maintainers, reviewers, security specialists and domain experts for help |
| Myths around treatment | Challenge engineering quick-fix myths: green CI, raw test counts, linters and generated patches are not proof of correctness |

The last row is a metaphorical expansion, not a claim about the video’s missing middle or its actual discussion of antidepressants.

## Support model

1. **People:** reachable maintainers, multiple owners for critical areas, code review and clear hand-offs.
2. **Maintenance capacity:** protected time for refactoring, dependency updates, documentation and flaky-test repair.
3. **Engineering practice:** reproducible builds, focused tests, CI and actionable diagnostics.
4. **Expert review:** architecture/security review, release gates and tested recovery plans.

## Measurable signals

The local scanner reports evidence such as source/test/documentation/workflow inventory, ownership files, maintenance markers, large source units and dependency manifests. It uses bounded, read-only inspection.

It deliberately does not create a single “mental health score”. A missing file is a prompt for human review, not a verdict. TODO/FIXME/HACK markers are context-dependent; a green CI run or a high test count does not prove correctness.

## Safety and privacy boundaries

- No diagnosis, human screening, contributor risk score or productivity ranking.
- No inference from contributor activity, commit timing, language, audio, hardware or device telemetry.
- No automatic source edits, dependency updates, deletion or remediation.
- No uploading project files or telemetry; scans run locally and read-only.
- No claim that static inventory proves correctness, security or maintainability.

## Usage

See the repository-specific command below. The common data contract is `data/code-wellbeing/codebase-wellbeing.v1.json`, validated by `schemas/codebase-wellbeing.v1.schema.json`.

## Repository implementation

Run the built-in `code-wellbeing` plugin from the existing plugin host; it inventories the configured workspace without touching audio buffers.
