# CLIFUEL

cliFuel is a command line fuel price finder for Italy

This program is based on open data published by Ministero dello sviluppo economico.
The database URL is:

- [anagrafica_impianti_attivi.csv](https://www.mise.gov.it/images/exportCSV/anagrafica_impianti_attivi.csv)
- [prezzo_alle_8.csv](https://www.mise.gov.it/images/exportCSV/prezzo_alle_8.csv)

## Examples

- Simple search

```
$ ./cliFuel -q roma -s
```

<center>
    <img src="img/search-only.png" width="100%" />
</center>

- Search for best price

```
$ ./cliFuel -q roma
```

<center>
    <img src="img/best-price.png" width="100%" />
</center>

## Libraries

1. https://curl.haxx.se/libcurl/c/libcurl.html
2. https://sourceforge.net/projects/cccsvparser/
3. https://github.com/rxi/log.c
4. https://github.com/rxi/map
5. https://github.com/doches/progressbar.git

## License

[![GNU GPLv3 Image](https://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl-3.0.en.html)

This application is Free Software: You can use, study share and improve it at your
will. Specifically you can redistribute and/or modify it under the terms of the
[GNU General Public License](https://www.gnu.org/licenses/gpl.html) as
published by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
