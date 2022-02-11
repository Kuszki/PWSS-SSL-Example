[![CMake build](https://github.com/Kuszki/PWSS-SSL-Example/actions/workflows/cmake.yml/badge.svg)](https://github.com/Kuszki/PWSS-SSL-Example/actions/workflows/cmake.yml)

# PWSS-SSL-Example
Przykładowy czat z wykorzystaniem SSL. Połaczeni klienci mają mozliwość wysyłania i odbioru wiadomości od wszystkich połączonych klientów.

Połączenie z serwerem jest szyfrowane z wykorzystaniem `OpenSSL`. Zarówno klient, jak i serwer, wymagają zaufanego certyfikatu. Nazwy klientów wyświetlane są na podstawie nazwy zawartej w certyfikacie.

W celu nawiązania połączenia należy uruchomić serwer i klientów zadając w parametrach programu ścieżki plików z kluczem i certyfikatem oraz parametry połaczenia. Parsowanie argumentów przedstawiono z wykorzystaniem `Boost::Program_options`.

Projekt stanowi przykład w ramach przedmiotu `Programowanie w Środowisku Sieciowym`. W celu zbudowania projektu należy wykorzystać narzędzie `CMake`.

## Generowanie certyfikatów
W celu wygenerowania certyfikatów dla serwera i klientów należy:

1) Utworzyć klucz prywatny certyfikatu głównego:
> openssl genrsa -out rootCA.key 4096

2) Utworzyć certyfikat główny:
> openssl req -x509 -new -nodes -key rootCA.key -sha256 -out rootCA.crt

3) Utworzyć klucz prywatny serwera:
> openssl genrsa -out server.key 2048

4) Utworzyć certyfikat serwera:
> openssl req -new -sha256 -key server.key -out server.csr

5) Podpisać certyfikat serwera kluczem głównym:
> openssl x509 -req -in server.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -out server.crt -sha256

Kroki 3, 4, 5 należy powtarzać w celu utworzenia kolejnych certyfiaktów (dla klientów):
> openssl genrsa -out client.key 2048
> 
> openssl req -new -sha256 -key client.key -out client.csr
> 
> openssl x509 -req -in client.csr -CA rootCA.crt -CAkey rootCA.key -CAcreateserial -out client.crt -sha256
