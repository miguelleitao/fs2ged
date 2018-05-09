# fs2ged
Convert FamilySearch export format files to GEDCOM

## Context
__getmyancestors__ from *daveneeley* (https://github.com/daveneeley/getmyancestors.git) is a Python script that claims to download family trees in GEDCOM format from FamilySearch. As far of my experiment, it produces usable data but not in valid GEDCOM format. 
__fs2ged__ is the conversion tool that allows producting valid GEDCOM files from the output of __getmyancestors__. Produced GEDCOM files can then be imported directly into Gramps or other geneanology software.

## Usage

    make
    ./fs2ged jfk.fs jfk.ged
 

## Other tools
__getmyancestors__ from *Linekio* (https://github.com/Linekio/getmyancestors.git) is a fork that downloads family trees in GEDCOM format from FamilySearch. It is a valuable solution that replaces both *daveneely*'s script and __fs2ged__.
I did not try the __getmyancestors__ version from *freeseek* (https://github.com/freeseek/getmyancestors.git)


