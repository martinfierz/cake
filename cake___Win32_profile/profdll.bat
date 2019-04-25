COPY %1.dll save
PREP /OM %1.dll
COPY %1._ll %1.dll
PROFILE /I %1 /O %1 %2 %3 %4 %5 %6
COPY save %1.dll
PREP /M %1
PLIST %1

