[![CMake build](https://github.com/Kuszki/PWSS-SSL-Example/actions/workflows/cmake.yml/badge.svg)](https://github.com/Kuszki/PWSS-SSL-Example/actions/workflows/cmake.yml)

# PWSS-SSL-Example
Przykładowy czat z wykorzystaniem SSL. Połaczeni klienci mają mozliwość wysyłania i odbioru wiadomości od wszystkich połączonych klientów.

Połączenie z serwerem jest szyfrowane z wykorzystaniem `OpenSSL`. Zarówno klient, jak i serwer, wymagają zaufanego certyfikatu. Nazwy klientów wyświetlane są na podstawie nazwy zawartej w certyfikacie.

W celu nawiązania połączenia należy uruchomić serwer i klientów zadając w parametrach programu ścieżki plików z kluczem i certyfikatem oraz parametry połaczenia. Parsowanie argumentów przedstawiono z wykorzystaniem `Boost::Program_options`.

Projekt stanowi przykład w ramach przedmiotu `Programowanie w Środowisku Sieciowym`. W celu zbudowania projektu należy wykorzystać narzędzie `CMake`.

## Działanie serwera

Serwer nasłuchuje połaczeń na wybranym interfejsie i porcie. Jeśli serwer zostanie zainicjalizowany z podaniem ścieżki do certyfikatu i klucza prywatnego będzie on wymagał szyfrowania połączeń przychodzących i będzie weryfikował certyfikaty klientów. W przypadku posiadania jedynie certyfikatu "self-signed" należy przekazać do serwera certyfikat CA, którym podpisano certyfikaty klientów.

Metoda "loop" serwera pozwala wykonać pętlę obsługi klientów - automatycznie akceptując nowe połaczenia. Metoda zwraca zbiory opisujace nowych klientów, klientów gotowych do odczytu i klientów gotowych do zapisu. Należy zamodzielnie zaimplementować obsługę klientów zgodnie z potrzebą - podobnie należy zaimplementować obsługę zamykania połączenia i ręcznie wywołać metodę "close" dla wybranego id klienta.

Po przechwyceniu sygnału z systemu operacyjnego metoda "loop" kończy pracę zwracając błąd - oznacza to, że należy najprawdopodobniej zakończyć program. Destruktor serwera zamknie wszystkie połączenia i zwolni związane z nimi zasoby.

Serwer rozpoznaje nazwę klienta bezpośrednio z treści certyfikatu - wyświetla pole "CN". W przypadku połaczeń nieszyfrowanych wyświetlany jest adres IP klienta.

Przykładowa implementacja serwera z wykorzystaniem dostarczonej klasy jest umieszczona w pliku `srvmain.cpp`. Serwer odbiera od klientów dane i wysyła je do wszystkich pozostałych klientów.

## Działanie klienta

Klient umożliwia nawiązanie połączenia z wybranym serwerem. Jeśli klient zostanie zainicjalizowany z podaniem ścieżki do certyfikatu i klucza prywatnego będzie on wymagał szyfrowania połączenia i będzie weryfikował certyfikat serwera. W przypadku posiadania jedynie certyfikatu "self-signed" należy przekazać do programu certyfikat CA, którym podpisano certyfikat serwera.

Implementacja wykorzystania klasy klienta, zawarta w pliku `climain.cpp`, wykorzystuje jeden wątek. Nieblokujaca obsługa przychodzących danych z serwera oraz wejścia terminala została zrealizowana za pomocą funkcji "poll". Po przechwyceniu sygnału z systemu operacyjnego metoda "loop" kończy pracę zwracając błąd - oznacza to, że należy zakończyć program. Destruktor serwera zamknie wszystkie połączenia i zwolni związane z nimi zasoby.

## Generowanie certyfikatów

### Przy użyciu `openssl`

1) Utworzyć klucz prywatny certyfikatu głównego:
> openssl genrsa -out rootCA.key 4096

2) Utworzyć certyfikat główny:
> openssl req -x509 -new -nodes -key rootCA.key -sha256 -out rootCA.crt

3) Utworzyć klucz prywatny serwera:
> openssl genrsa -out server.key 2048

4) Utworzyć informacje o certyfikacie serwera:
> openssl req -new -sha256 -key server.key -out server.csr

5) Utworzyć certyfikat serwera podpisany kluczem głównym:
> openssl x509 -req -in server.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -out server.crt -sha256

Kroki 3, 4, 5 należy powtarzać w celu utworzenia kolejnych certyfiaktów (dla klientów):
> openssl genrsa -out client.key 2048
>
> openssl req -new -sha256 -key client.key -out client.csr
>
> openssl x509 -req -in client.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -out client.crt -sha256

### Przy użyciu `easy-rsa`

1) Zainicjować folder pki:
> easy-rsa init-pki

2) Utworzyć certyfikat główny:
> easy-rsa build-ca nopass

3) Utworzyć certyfikat serwera:
> easy-rsa build-server-full nazwa nopass

4) Utworzyć certyfikat dla klienta:
> easy-rsa build-client-full nazwa nopass

### Informacje o `easy-rsa`

W pliku `vars` znajduje się konfiguracja generowanych certyfikatów, a jej wartości domyślne zależą od opiekuna pakietu `easy-rsa`. Generowane certyfikaty serwera i klientów są automatycznie podpisywane certyfikatem głównym. Wszystkie wystawione certyfikaty znajdują się w katalogu `issued`, a powiązane z nimi klucze prywatne w katalogu `private`.

Parametr `nazwa` oznacza nazwę podmiotu, dla którego wystawiany jest certyfikat (pole `CN - Common Name`). Zwykle jest to imię i nazwisko. Parametr `nopass` jest opcjonalny i oznacza, że certyfikat nie będzie zabezpieczony hasłem (przydatne, gdy zostaje on wczytany przez program np. serwer).

W niektórych dystrubycjach (np. `Debian` krok pierwszy musi być poprzedzony utworzeniem odpowiedniej struktury katalogów: `make-cadir katalog && cd katalog`. Dodatkowo program `easy-rsa` może nazywać się `easyrsa` i może nie występować w `$PATH` - wtedy wywołanie `make-cadir` w utworzonym folderze umieści dowiązanie symboliczne do właściwego programu.
