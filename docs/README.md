# Documentation

These pages are organised by the question each one answers. A page does one
job, so a reader can tell from the path which page to open, and no single page
tries to do several things at once.

| I want to… | Read |
| :--- | :--- |
| Understand the project and why it is built this way | [explanation/how-it-works.md](explanation/how-it-works.md) |
| Understand the code that exists, line by line | [explanation/what-the-code-does.md](explanation/what-the-code-does.md) |
| Know what works today and what is next | [status.md](status.md) |
| Look up a term like "register" or "V-Blank" | [reference/glossary.md](reference/glossary.md) |
| Know why a design choice was made | [decisions/](decisions/README.md) |
| Contribute to the project | [../CONTRIBUTING.md](../CONTRIBUTING.md) |

There is no tutorial yet, and no API reference. Both require something that can
actually run a game, which does not exist yet.

## Layout

```text
docs/
├── README.md          this page
├── status.md          what works now, what is next
├── explanation/       why the project and the code are the way they are
├── reference/         fixed look-up material
└── decisions/         one file per design decision
```text

Two files sit at the repository root — [what-is-possible.md](../what-is-possible.md)
and [the milestone plan](../virtual_8_bit_cpu_browser_hardware_architecture_plan.md).
They were written before any code existed, so they read as research notes rather
than documentation. Their reasoning is kept, but the pages you actually want are
above.
