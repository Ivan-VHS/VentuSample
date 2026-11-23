# Documentation control

| # | Rule |
|---:|---|
| 1 | implemented claims link to code/tests |
| 2 | distinguish objective metrics from subjective sound |
| 3 | document accepted formats and numeric limits |
| 4 | version manifest/schema changes |
| 5 | state unsupported formats and known limitations |
| 6 | describe sample-rights responsibilities |
| 7 | verify links, commands and examples before release |

## Internal information architecture

| # | Rule | Acceptance |
|---:|---|---|
| 8 | Topic ownership | Every important page belongs to one primary topic hub and is listed in the central map. |
| 9 | Navigation depth | Important audio, DSP and Studio contracts are reachable within three to four steps from the root README. |
| 10 | No orphan pages | Each important page has at least two meaningful inbound paths, normally the central catalog and its topic hub. |
| 11 | Contextual links | Connect pages when an audio workflow, data format, plugin lifecycle or runtime boundary actually relates them. |
| 12 | Descriptive anchors | Name the target module or contract; avoid vague labels and keyword-stuffed repetition. |
| 13 | Scope consistency | README, hubs and detailed documents agree on supported formats, numeric limits, sample rights, offline behavior and maturity. |
| 14 | Canonical targets | Use direct relative links within this repository and canonical HTTPS links across repositories; avoid avoidable redirects and duplicate targets. |
| 15 | Breadcrumbs | Topic hubs link to the documentation home and parent map; add equivalent breadcrumbs to new deep-level pages. |
| 16 | Link density | Keep links contextual and useful; avoid repetitive link blocks and link-heavy paragraphs. |
| 17 | New-page propagation | Update the topic hub and central catalog in the same change and link the page from relevant existing material. |
| 18 | Link maintenance | Check changed paths and periodically audit broken or obsolete links. |
| 19 | Measurement | Measure navigation only on supported published surfaces with privacy-respecting analytics; do not claim GitHub Markdown click telemetry. |
| 20 | Accessibility | Use descriptive anchors, same-tab internal navigation by default and link styling that remains usable on small screens. |

The [information architecture map](INFORMATION_ARCHITECTURE.md) is the navigation source for this repository. It does not override audio-processing evidence or claim ownership of user-provided samples and generated audio.
