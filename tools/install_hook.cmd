set script="commit-msg"

for /D %%f in (..\.git\hooks\ ..\.git\modules\Lib\* ..\.git\modules\LibCert\*) do (
  copy %script% %%f
)

pause
