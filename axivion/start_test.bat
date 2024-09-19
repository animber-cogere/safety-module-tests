
set BAUHAUS_CONFIG=%~dp0\axivion_config.json

cafeCC -o BracesTest.ir BracesTest.c
axivion_analysis --ir BracesTest.ir --rule CodingStyle-WhitesmithBraces
