# AI Handoff: Command Definition Completion

## Objective

Replace minimal command-definition scaffolds with accurate, usable JSON definitions containing command-specific options, positional arguments, types, defaults, choices, dependencies, and groups. Keep `commands/COMMANDS.txt` as a names-only inventory.

## Current repository state

- Command definition files: 148.
- Unique command names and `commands/COMMANDS.txt` entries: 147.
- The one-file difference is the intentional duplicate command name `chmod` for Linux and macOS.
- `commands/COMMANDS.txt` must contain exactly one bare command name per line and no headings, counts, paths, or descriptions.
- `PROJECT_REPORT.md` is intentionally ignored by Git and records the current totals.

## Work completed in this pass

Upgraded these scaffolded definitions with typed, command-specific options:

- `commands/linux/core/bash.json`
- `commands/linux/core/java.json`
- `commands/linux/core/mysql.json`
- `commands/cross-platform/python3.json`
- `commands/linux/core/basename.json`
- `commands/linux/core/date.json`
- `commands/linux/core/dirname.json`
- `commands/linux/core/file.json`
- `commands/linux/core/hostname.json`
- `commands/linux/core/id.json`
- `commands/linux/core/realpath.json`
- `commands/linux/core/readlink.json`
- `commands/linux/core/stat.json`
- `commands/linux/core/whoami.json`
- `commands/linux/core/apt-get.json`
- `commands/linux/core/base64.json`
- `commands/linux/core/jq.json`
- `commands/linux/core/mktemp.json`
- `commands/linux/core/pgrep.json`

Sources used:

- Installed command help output (`bash --help`, `java --help`, `mysql --help`, `python3 --help`).
- GNU Bash manual: https://www.gnu.org/software/bash/manual/html_node/Invoking-Bash.html
- Java launcher reference: https://docs.oracle.com/en/java/javase/26/docs/specs/man/java.html
- MariaDB/MySQL client reference: https://mariadb.com/docs/server/clients-and-utilities/mariadb-client
- Python command-line documentation: https://docs.python.org/3/using/cmdline.html

## Important audit finding

Most remaining definitions are scaffolds: before this pass, 865 files had exactly one generic `--help` option and 25 had no options. The generated entries generally have a generic repeatable positional argument and a generic help flag. They are schema-valid but are not yet thorough command references.

## Recommended continuation order

1. Upgrade high-use/core commands first: `awk`, `cat`, `cp`, `curl`, `find`, `grep`, `ls`, `mv`, `rm`, `sed`, `ssh`, `tar`, `wget`, `zip`, and `unzip`.
2. Upgrade developer/runtime tools: compilers, interpreters, package managers, database clients, container tools, and cloud CLIs.
3. Upgrade specialist/security/network tools.
4. For each command, use the installed executable’s `--help` and `man` page first. If unavailable, consult the official upstream manual for the relevant major version. Do not silently merge options from unrelated versions.
5. Record the command/version and source URL in this handoff or a future provenance field/document when a definition is upgraded.

## JSON rules

Follow the schema consumed by `core/json_parser.cpp` and documented in `docs/json_structure.md`. Every file should include `meta`, `positional`, `groups`, `mutex_groups`, `subcommands`, and `options`. Options should use the supported types (`BOOL`, `STRING`, `INT`, `FLOAT`, `ENUM`, `FILE_PATH`, `DIR_PATH`, `STRING_LIST`), correct separators, and stable unique IDs. Use `repeatable` only when the command accepts repeated occurrences or a comma-separated UI value is appropriate.

The current command builder emits values in JSON option order and invokes commands through a shell string. Avoid definitions that require shell quoting semantics the builder cannot represent. Be especially careful with options whose syntax is a prefix (`-Dname=value`), optional values, bundled short flags, or subcommands; model them conservatively or document the limitation.

## Verification commands

```bash
for f in $(find commands -type f -name '*.json'); do jq empty "$f"; done
diff -u \
  <(find commands -type f -name '*.json' -printf '%f\\n' | sed 's/\\.json$//' | sort -u) \
  <(sort -u commands/COMMANDS.txt)
test "$(wc -l < commands/COMMANDS.txt)" -eq 147
git diff --check
```

Do not commit changes. Preserve the existing `.gitignore` rules and keep `PROJECT_REPORT.md` ignored.
