# Documentation

These pages are organised by **what the reader needs**, following the
[Diátaxis](https://diataxis.fr/) framework: a page should do one of four jobs,
and never several at once.

| I want to… | Read | Mode |
| :--- | :--- | :--- |
| Understand what we're building and why | [explanation/how-it-works.md](explanation/how-it-works.md) | Explanation |
| Understand the code that exists, line by line | [explanation/what-the-code-does.md](explanation/what-the-code-does.md) | Explanation |
| Know what works today and what's next | [status.md](status.md) | Status |
| Look up a term like "V-Blank" or "register" | [reference/glossary.md](reference/glossary.md) | Reference |
| Know why a choice was made | [decisions/](decisions/README.md) | ADR |
| Contribute code | [../CONTRIBUTING.md](../CONTRIBUTING.md) | How-to |

There is no **tutorial** yet, and no **API reference**: the public API does not
exist, so documenting it would be inventing it. Both arrive once something can
actually run a game.

## Layout

```text
docs/
├── README.md          this page
├── status.md          what works now, what's next
├── explanation/       why the thing is the way it is
├── reference/         look-up material
└── decisions/         one file per design decision (ADRs)
```

The two files at the repository root — [what-is-possible.md](../what-is-possible.md)
and [the milestone plan](../virtual_8_bit_cpu_browser_hardware_architecture_plan.md) —
are research notes written before any code existed. They are kept for the
reasoning, not as documentation.
