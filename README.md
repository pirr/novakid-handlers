# Novakid handlers

## Database handler

### opportunities:

    - download a database dump from nexus
    - restore database in docker

### configure:

    1. create .config
    2. add params

    backup_url=https://nexus/repository/path/to/backup
    usrpwd=username:password
    outfile=path/to/outfile

### make:
    restore_db - restore db from backup (outfile).
    restore_db_download - download last backup and restore db.