[![CMake build](https://github.com/Kuszki/PWSS-SSL-Example/actions/workflows/cmake.yml/badge.svg)](https://github.com/Kuszki/PWSS-SSL-Example/actions/workflows/cmake.yml)

# PWSS-SSL-Example
Przykładowy czat z wykorzystaniem SSL. Połączeni klienci mają możliwość wysyłania i odbioru wiadomości od wszystkich połączonych klientów.

Połączenie z serwerem jest szyfrowane z wykorzystaniem `OpenSSL`. Zarówno klient, jak i serwer, wymagają zaufanego certyfikatu. Nazwy klientów wyświetlane są na podstawie nazwy zawartej w certyfikacie.

W celu nawiązania połączenia należy uruchomić serwer i klientów zadając w parametrach programu ścieżki plików z kluczem i certyfikatem oraz parametry połaczenia. Parsowanie argumentów przedstawiono z wykorzystaniem `Boost::Program_options`.

Projekt stanowi przykład w ramach przedmiotu `Programowanie w Środowisku Sieciowym`. W celu zbudowania projektu należy wykorzystać narzędzie `CMake`.

## Działanie serwera

Serwer nasłuchuje połączeń na wybranym interfejsie i porcie. Jeśli serwer zostanie zainicjalizowany z podaniem ścieżki do certyfikatu i klucza prywatnego będzie on wymagał szyfrowania połączeń przychodzących i będzie weryfikował certyfikaty klientów. W przypadku posiadania jedynie certyfikatu "self-signed" należy przekazać do serwera certyfikat CA, którym podpisano certyfikaty klientów.

Metoda "loop" serwera pozwala wykonać pętlę obsługi klientów - automatycznie akceptując nowe połączenia. Metoda zwraca zbiory opisujace nowych klientów, klientów gotowych do odczytu i klientów gotowych do zapisu. Należy samodzielnie zaimplementować obsługę klientów zgodnie z potrzebą - podobnie należy zaimplementować obsługę zamykania połączenia i ręcznie wywołać metodę "close" dla wybranego id klienta.

Po przechwyceniu sygnału z systemu operacyjnego metoda "loop" kończy pracę zwracając błąd - oznacza to, że należy najprawdopodobniej zakończyć program. Destruktor serwera zamknie wszystkie połączenia i zwolni związane z nimi zasoby.

Serwer rozpoznaje nazwę klienta bezpośrednio z treści certyfikatu - wyświetla pole "CN". W przypadku połaczeń nieszyfrowanych wyświetlany jest adres IP klienta.

Przykładowa implementacja serwera z wykorzystaniem dostarczonej klasy jest umieszczona w pliku `srvmain.cpp`. Serwer odbiera od klientów dane i wysyła je do wszystkich pozostałych klientów.

## Działanie klienta

Klient umożliwia nawiązanie połączenia z wybranym serwerem. Jeśli klient zostanie zainicjalizowany z podaniem ścieżki do certyfikatu i klucza prywatnego będzie on wymagał szyfrowania połączenia i będzie weryfikował certyfikat serwera. W przypadku posiadania jedynie certyfikatu "self-signed" należy przekazać do programu certyfikat CA, którym podpisano certyfikat serwera.

Implementacja wykorzystania klasy klienta, zawarta w pliku `climain.cpp`, wykorzystuje jeden wątek. Nieblokująca obsługa przychodzących danych z serwera oraz wejścia terminala została zrealizowana za pomocą funkcji "poll". Po przechwyceniu sygnału z systemu operacyjnego metoda "loop" kończy pracę zwracając błąd - oznacza to, że należy zakończyć program. Destruktor serwera zamknie wszystkie połączenia i zwolni związane z nimi zasoby.

## Generowanie certyfikatów

### Przy użyciu `openssl`

1) Utworzyć klucz prywatny certyfikatu głównego:
``` bash
openssl genrsa -out rootCA.key 4096
```

2) Utworzyć certyfikat główny:
``` bash
openssl req -x509 -new -nodes -key rootCA.key -sha256 -out rootCA.crt
```

3) Utworzyć klucz prywatny serwera:
``` bash
openssl genrsa -out server.key 2048
```

4) Utworzyć informacje o certyfikacie serwera:
``` bash
openssl req -new -sha256 -key server.key -out server.csr
```

5) Utworzyć certyfikat serwera podpisany kluczem głównym:
``` bash
openssl x509 -req -in server.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -out server.crt -sha256
```

Kroki 3, 4, 5 należy powtarzać w celu utworzenia kolejnych certyfiaktów (dla klientów):
``` bash
openssl genrsa -out client.key 2048
openssl req -new -sha256 -key client.key -out client.csr
openssl x509 -req -in client.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -out client.crt -sha256
```

Certyfikaty wraz z kluczami prywatnymi mogą być umieszczane w jednym pliku. Ten sposób ułatwia dystrybucję certyfikatów oraz umożliwia ich importowanie w przeglądarkach internetowych. Należy w tym celu wygenerować plik w formacie `pkcs12`:
``` bash
openssl pkcs12 -export -out nazwa.p12 -inkey klucz.key -in certyfikat.crt
```

Stosując `openssl` istnieje również możliwość dołączenia do pliku `pkcs12` certyfikatu głównego, którym podpisano pozostałe certyfikaty. W ten sposób plik wynikowy zawiera wszystkie niezbędne pliki: certyfikat główny, certyfikat klienta oraz jego klucz prywatny. Aby to osiągnąc stosuje się:
``` bash
openssl pkcs12 -export -out nazwa.p12 -inkey klucz.key -in certyfikat.crt -certfile ca.crt
```

### Przy użyciu `easy-rsa`

1) Zainicjować folder pki:
``` bash
easyrsa init-pki
```

2) Utworzyć certyfikat główny:
``` bash
easyrsa build-ca nopass
```

3) Utworzyć certyfikat serwera:
``` bash
easyrsa build-server-full nazwa nopass
```

4) Utworzyć certyfikat dla klienta:
``` bash
easyrsa build-client-full nazwa nopass
```

5) Opcjonalnie eksportować certyfikaty do pliku `pkcs12`:

``` bash
easyrsa export-p12 nazwa
```

### Informacje o `easy-rsa`

W pliku `vars` znajduje się konfiguracja generowanych certyfikatów, a jej wartości domyślne zależą od opiekuna pakietu `easy-rsa`. Generowane certyfikaty serwera i klientów są automatycznie podpisywane certyfikatem głównym. Wszystkie wystawione certyfikaty znajdują się w katalogu `issued`, a powiązane z nimi klucze prywatne w katalogu `private`.

Parametr `nazwa` oznacza nazwę podmiotu, dla którego wystawiany jest certyfikat (pole `CN - Common Name`). Zwykle jest to imię i nazwisko. Parametr `nopass` jest opcjonalny i oznacza, że certyfikat nie będzie zabezpieczony hasłem (przydatne, gdy zostaje on wczytany przez program np. serwer).

W niektórych dystrubycjach (np. `Debian` krok pierwszy musi być poprzedzony utworzeniem odpowiedniej struktury katalogów: `make-cadir katalog && cd katalog`. Dodatkowo program `easy-rsa` może nazywać się `easyrsa` i może nie występować w `$PATH` - wtedy wywołanie `make-cadir` w utworzonym folderze umieści dowiązanie symboliczne do właściwego programu.

## Przykład uruchomienia programów

Klonowanie, konfigurowanie i budowanie projektu w oddzielnym katalogu:
``` bash
git clone https://github.com/Kuszki/PWSS-SSL-Example    # klonuj repozytorium
mkdir build-PWSS-SSL-Example                            # utwórz katalog budowania
cd build-PWSS-SSL-Example                               # przejdź do katalogu budowania
cmake ../PWSS-SSL-Example                               # konfiguruj projekt
cmake --build .                                         # zbuduj dla wybranej konfiguracji
```

Tworzenie certyfikatów (przykład dla `Debiana`):
``` bash
make-cadir certs                             # inicjuj katalog z certyfikatami
cd certs                                     # przejdź do zainicjowanego katalogu
./easyrsa init-pki                           # inicjuj strukturę pki
./easyrsa build-ca nopass                    # zbuduj certyfikat CA
./easyrsa build-server-full Serwer nopass    # zbuduj certyfikat serwera
./easyrsa build-client-full Cli_A nopass     # zbuduj certyfikat klienta A
./easyrsa build-client-full Cli_B nopass     # zbuduj certyfikat klienta B
```

Uruchamianie programów (z katalogu `build-PWSS-SSL-Example`):
``` bash
./SSL_serwer \                               # uruchom serwer
     --ca=certs/pki/ca.crt \                 # certyfikat główny
     --key=certs/pki/private/Serwer.key \    # klucz prywatny
     --cert=certs/pki/issued/Serwer.crt      # certyfikat serwera

./SSL_klient \                               # uruchom klienta
     --ca=certs/pki/ca.crt \                 # certyfikat główny
     --key=certs/pki/private/Cli_A.key \     # klucz prywatny
     --cert=certs/pki/issued/Cli_A.crt       # certyfikat klienta

./SSL_klient \                               # uruchom klienta
     --ca=certs/pki/ca.crt \                 # certyfikat główny
     --key=certs/pki/private/Cli_B.key \     # klucz prywatny
     --cert=certs/pki/issued/Cli_B.crt       # certyfikat klienta
```

Działanie serwera:
![Serwer](https://user-images.githubusercontent.com/6035437/210174752-ae0edd13-c09d-4a64-9991-4f201c135cde.png)

Klient pierwszy:
![Klient_A](https://user-images.githubusercontent.com/6035437/210174767-edd180f3-21c0-4990-914e-f2529c93ec47.png)

Klient drugi:
![KlientBB](https://user-images.githubusercontent.com/6035437/210174773-52eef4e4-3046-4657-aff8-06815a0a0abc.png)

Działanie programów kończy sygnał `SIGINT` wywołany kombinacją `Ctrl+C` w terminalu. Zakończenie pracy serwera rozłącza wszystkich klientów, którzy w efekcie wychodzą z funkcji `main` wraz z kodem błędu `-1` (powłoka `fish` wyświetla na czerwono liczbę `255`).
