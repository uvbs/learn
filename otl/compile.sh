ORACLE_HOME=/opt/oracle/product/11.2.0/dbhome_1
g++ -g -I. -I$ORACLE_HOME/rdbms/public/ -L$ORACLE_HOME/lib/ -lclntsh -ldl -o otloracle  main.cpp


