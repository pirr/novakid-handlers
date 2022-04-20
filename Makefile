BACKUP_URL := $(shell grep 'backup_url=*' .config | awk '{ split($$0, a, "=") ; print a[2] }')
USRPWD := $(shell grep 'usrpwd=*' .config | awk '{ split($$0, a, "=") ; print a[2] }')
OUTFILE := $(shell grep 'outfile=*' .config | awk '{ split($$0, a, "=") ; print a[2] }')
DB_HANDLER_FILE = ./out/db_handler.o

config:
	cc test_parse.c conf_parser.c -o test_parse -lyaml

clean:
	-rm -f *.o

db_handler: clean
	$(CC) utils.c downloader.c db_handler.c -o $(DB_HANDLER_FILE) -lcurl

restore_db:
	$(DB_HANDLER_FILE)

restore_db_download:
	$(DB_HANDLER_FILE) -d -u $(BACKUP_URL) -p $(USRPWD)  -o $(OUTFILE)
