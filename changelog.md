


### First firmware revision for Ampaire's PEEK-300.
commit 90d899a (tag: v2.1.2.1_A1, origin/main, main)
Author: Joshua Jacobs <joshua.jacobs@ampaire.com>
Date:   Thu Feb 29 14:47:23 2024 -0800

    chore: update readme

commit 37861ee
Author: Joshua Jacobs <joshua.jacobs@ampaire.com>
Date:   Wed Feb 28 09:56:48 2024 -0800

    fix: disable preheat before leveling. conflict with preheat before probing

commit dc25b9f
Author: Joshua Jacobs <joshua.jacobs@ampaire.com>
Date:   Tue Feb 27 17:50:29 2024 -0800

    feat: enable Adaptive Step Smoothing
           enable Arc Support

commit 7190ac2
Author: Joshua Jacobs <joshua.jacobs@ampaire.com>
Date:   Tue Feb 27 16:54:21 2024 -0800

    feat: enable PID per hotend
            enable Distinct E Factors
            enable Multiple Probing and Extra Probing
            decrease probing clearances
            enable M48 repeatability test
            enable preheat before probing
            enable preheat before leveling
            increase number of leveling grid points
            enable extrapolate beyond grid


### Upload of standard CreatBot modified Marlin v2.1.2.1
commit b69f368
Author: Joshua Jacobs <joshua.jacobs@ampaire.com>
Date:   Tue Feb 27 13:31:47 2024 -0800

    chore: replace config files with those supplied by CreatBot for the PEEK-300

commit e555b90 (tag: v2.1.2.1_CB)
Author: Joshua Jacobs <joshua.jacobs@ampaire.com>
Date:   Tue Feb 27 10:21:33 2024 -0800

    chore: Add CreatBot Firmware to repo and update Readme

commit 5c3549b
Author: Joshua Jacobs <josh.jacobs@ampaire.onmicrosoft.com>
Date:   Tue Feb 27 10:06:35 2024 -0800

    first commit




### General Info

# command for updating changelog:
git log <last main commit>..main --decorate=short --abbrev-commit --first-parent main | cat - changelog.md > temp && mv temp changelog.md


