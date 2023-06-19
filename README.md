Archival Note: Markdown is a sub-optimal standard that should die. I'm not going to be supporting this tool in the future, so as of 05/19/2023, this branch is archived.

### m2h

Markdown 2(to) HTML

**Translation Notes**:
- Use `$`/`$$` to denote LaTex; `\(`/`\)` and `\[`/`\]` are treated as escaped sequences.
- Only `---` is treated as a horizontal line.
- Only `*` is treated as Bold/Itallic. Nested Bold/Itallics are supported.
- Double newlines are not ignored.


**Usage**:
m2h uses standard cin/cout.

To read `test.md` and produce `test.html` (with executable as `m2h.out`);
`cat test.md | m2h.out > test.html`

---

Originally part of [s3g](https://github.com/u20n/s3g), was isolated to better promote interoperability.
