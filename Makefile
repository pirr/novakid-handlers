config:
	cc test_parse.c conf_parser.c -o test_parse -lyaml

db_handler:
	cc utils.c downloader.c db_handler.c -o db_handler -lcurl
