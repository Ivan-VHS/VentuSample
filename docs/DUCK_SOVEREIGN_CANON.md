# Shared desktop design guidance

Common principles: clear product identity, task-first navigation, visible state, keyboard focus, accessible contrast and restrained white/blue/red signaling. Product-specific palettes and native toolkit behavior remain defined in each app's design authority; do not force identical colors or widgets across Qt, Tk and Win32.

## 2026 semantic extension

The original six brand colors remain canonical. Qt and other native desktop themes may additionally use this neutral and semantic extension for calm work surfaces, readable dark mode, selection and accessible focus. Blue remains action/focus; red remains critical/destructive.

| Token | Light | Dark |
|---|---|---|
| Workspace | #F4F8FD | #071A2D |
| Surface | #FFFFFF | #0F2A45 |
| Elevated | #EAF1FA | #173A5E |
| Primary text | #122C45 | #E8EDF2 |
| Secondary | #314B66 | #D0DCE8 |
| Muted | #4D6177 | #B3C4D6 |
| Action blue | #004E98 | #70A5FF |
| Focus / hover blue | #1A5FDB | #8AB6FF |
| Critical red | #B5122D | #FF7B86 |
| Critical hover | #9C1026 | #FF9AA2 |
| Border | #CAD8E8 | #31516F |
| Selection | #D9E8FF | #244F82 |

The source validator accepts these colors and continues to verify high-contrast text pairs. Gradients, color-only states and decorative motion remain outside the canon.
