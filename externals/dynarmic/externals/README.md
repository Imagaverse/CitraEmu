This repository uses subtrees to manage some of its externals.

## Initial setup

```
git remote add externals-catch https://github.com/catchorg/Catch2.git --no-tags
git remote add externals-fmt https://github.com/fmtlib/fmt.git --no-tags
git remote add externals-mcl https://github.com/merryhime/mcl.git --no-tags
git remote add externals-oaknut https://github.com/merryhime/oaknut.git --no-tags
git remote add externals-robin-map https://github.com/Tessil/robin-map.git --no-tags
git remote add externals-xbyak https://github.com/herumi/xbyak.git --no-tags
git remote add externals-zycore https://github.com/zyantific/zycore-c.git --no-tags
git remote add externals-zydis https://github.com/zyantific/zydis.git --no-tags
```

## Updating

Change `<ref>` to refer to the appropriate git reference.

```
git fetch externals-catch
git fetch externals-fmt
git fetch externals-mcl
git fetch externals-oaknut
git fetch externals-robin-map
git fetch externals-xbyak
git fetch externals-zycore
git fetch externals-zydis
git subtree pull --squash --prefix=externals/catch externals-catch <ref>
git subtree pull --squash --prefix=externals/fmt externals-fmt <ref>
git subtree pull --squash --prefix=externals/mcl externals-mcl <ref>
git subtree pull --squash --prefix=externals/oaknut externals-oaknut <ref>
git subtree pull --squash --prefix=externals/robin-map externals-robin-map <ref>
git subtree pull --squash --prefix=externals/xbyak externals-xbyak <ref>
git subtree pull --squash --prefix=externals/zycore externals-zycore <ref>
git subtree pull --squash --prefix=externals/zydis externals-zydis <ref>
```
