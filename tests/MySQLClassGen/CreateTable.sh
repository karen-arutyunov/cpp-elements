#!/bin/sh

ARGV="$@"
DB_CLIENT="ng-mysql"
DB_NAME="test"
DB_USER=root

if test "$ARGV" = "--recreate"; then
  echo "Droping $DB_NAME table ..."
  echo "drop table AllTypes;" | $DB_CLIENT --user=$DB_USER $DB_NAME
  
  if test $? -ne 0; then
    echo "failed"
    exit $?
  fi
fi

echo "Creating table AllTypes ..."
echo "create table if not exists AllTypes ( \
      id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, \
      url VARCHAR(255) NOT NULL UNIQUE KEY, \
      created TIMESTAMP NOT NULL, \
      creator_type ENUM ('C', 'M', 'U') NOT NULL DEFAULT 'M',
      creator_name VARCHAR(255) NOT NULL DEFAULT '', \
      activity DOUBLE NOT NULL DEFAULT '2.5', \
      density FLOAT DEFAULT '17.487139', \
      rate DECIMAL, \
      flags BIT(3) DEFAULT 3, \
      start_year YEAR DEFAULT 2005, \
      timeX DATETIME DEFAULT '2007-01-01 12:15:17', \
      rgb SET('red', 'green', 'blue'), \
      comment TEXT \
);" | $DB_CLIENT --user=$DB_USER $DB_NAME

if test $? -ne 0; then
  echo "failed"  
  exit $?
fi

echo "Filling table AllTypes ..."

query="insert ignore into AllTypes set \
 url='http://rss.news.yahoo.com/rss/topstories', created=NOW(), rgb='red'; \
 insert ignore into AllTypes set \
 url='http://rss.news.yahoo.com/rss/world', created=NOW();"

echo "$query" | $DB_CLIENT --user=$DB_USER $DB_NAME

if test $? -ne 0; then
  echo "failed"  
  exit $?
fi

echo "done"
