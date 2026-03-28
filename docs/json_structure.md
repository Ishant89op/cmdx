# Fields

| Field | Type | Purpose |
|---|---|---|
| `id` | string | unique key the parser and GUI use internally - never changes even if the flag changes |
| `flag` | string | the actual string written into the command |
| `label` | string | shown in the GUI next to the widget |
| `description` | string | tooltip or subtitle under the label |
| `type` | enum | determines which Qt widget to render |
| `required` | bool | red `*` label, Run button disabled until filled |
| `default` | any / null | pre-fills the widget on load |
| `separator` | `" "` / `"="` / `null` | `null` = BOOL flag (no value), `" "` = `-p 80`, `"="` = `--color=auto` |
| `depends_on` | id / null | widget is greyed out until the referenced option is enabled |
| `mutex_group` | id / null | only one option in the group can be active at a time |
| `choices` | array / null | populates `QComboBox` for ENUM type |
| `min` / `max` | int / null | bounds for `QSpinBox` and `QDoubleSpinBox` |
| `repeatable` | bool | adds an "add another" button - same flag used multiple times |
| `placeholder` | string / null | grey hint text inside `QLineEdit` |
| `group` | id | which collapsible section this option renders inside |
| `platforms` | array | on `meta` - which OSes load this JSON |
| `has_subcommands` | bool | parser reads `options[]` directly if false, `subcommands[].options` if true |

---

# Parser
```
load JSON
  |--- read meta
        |--- has_subcommands = false
        |     |--- parse options[] directly
        |           |--- for each option
        |                 |--- type=BOOL      → QCheckBox, separator=null
        |                 |--- type=STRING    → QLineEdit, separator=" "
        |                 |--- type=INT       → QSpinBox, separator=" "
        |                 |--- type=FLOAT     → QDoubleSpinBox
        |                 |--- type=ENUM      → QComboBox, separator="=" or " "
        |                 |--- type=FILE_PATH → QLineEdit + QFileDialog button
        |                 |--- type=DIR_PATH  → QLineEdit + QFileDialog button
        |                 |--- type=STRING_LIST → QLineEdit with comma hint
        |
        |--- has_subcommands = true
              |--- render subcommand selector first (QComboBox)
                    |--- on selection → parse that subcommand's options[]
                          |--- same tree as above