# Kahoot_clone
EKRAN GŁÓWNY:

Gracz łączy się do serwera i ma do wyboru trzy opcje Dołacz do gry, Stwórz 
nową grę, Rozpocznij istniejącą grę.

Dołacz do gry:

Gracz podaję nickname, ID gry, oraz PIN gry.

Po przekazaniu tych danych sprawdzna jest dostępność nickanemu w danej grze,
oraz status/istnienie danej gry. Gracz jest informowany o wszystkim 
odpowiednimi komunikatami

Gracz po wybraniu nicku trafia do lobby, w którym widzi bieżącą listę pokoi
i ilość graczy która jest w każdym pokoju.

Z lobby gracz może wejść do istniejącego pokoju lub założyć nowy. Gracz może
w każdej chwili wrócić z pokoju do lobby.

Jeśli w pokoju trwa gra, gracz dołącza jako zgadujący. Jeśli nie, gracz widzi
listę graczy którzy są w pokoju i czeka na rozpoczęcie gry.

Grę może rozpocząć gracz który najdłużej czeka w pokoju, pod warunkiem że w
pokoju jest przynajmniej trzech graczy.

Gracz który rozpoczął grę staje się pokazującym. Pokazujący wysyła do serwera
jakie hasło będzie pokazywać. Jeśli tego nie zrobi w określonym czasie,
kolejny z listy graczy staje się pokazującym.

Po przekazaniu hasła do serwera, kamera gracza pokazującego jest przesyłana
przez serwer do zgadujących (pozostałych graczy). Zgadujący widzą ile słów
jest w haśle.

Zgadujący mogą wpisywać pojedyncze słowa z odgadywanego hasła. Każde wpisane
słowo jest widziane przez wszystkich graczy. Jeśli słowo jest w haśle, gracz
który je odgadł dostaje punkt, a słowo jest wpisywane we właściwe miejsce w
haśle u wszystkich graczy. Jeśli kilku graczy wpisało to samo słowo (przed
wyświetleniem go na swoich ekranach), to każdy dostaje punkt.

Kiedy całe hasło zostało odgadnięte, lub kiedy upłynął określony czas, lub
kiedy rozłączył się pokazujący, kolejny gracz z listy graczy zostaje
pokazującym.

Gra trwa do momentu aż w pokoju jest przynajmniej trzech graczy. Gracze na
bieżąco widzą ranking graczy z punktami (ilością odgadniętych słów), włączając
w to też graczy którzy rozłączyli się z gry.
