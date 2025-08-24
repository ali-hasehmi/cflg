## Improvements
- [x] Convert default functions(e.g. flgset_int, flgset_bool) to macros
- [ ] Create a mechanism to customize output stream of library
- [ ] Create a mechanism to customize help message
- [ ] Add usage function to print arbitrary usage message before showing help
- [ ] Refactor flgset_parse function

## Bugs:
- [ ] Program doesn't differentiate between '-h' and "--h"
- [ ] If an arugment is forced on a bool flag(e.g. --verbose=forced_argument), program ignores the arugment instead of producing error
- [ ] There is no mechanism to inform parsers if an arugment has been forced
