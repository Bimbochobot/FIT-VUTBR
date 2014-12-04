/***************************************************************************/
/* Odstran tabulky a sekvencie, ktore mohli existovat pred spustenim.
/***************************************************************************/
drop table Objednavka cascade constraints purge;
drop table Odberatel cascade constraints purge;
drop table Varka cascade constraints purge;
drop table Caj cascade constraints purge;
drop table CajovaOblast cascade constraints purge;
drop table Dodavatel cascade constraints purge;
drop table PolozkaObjednavky cascade constraints purge;

drop sequence seq_cajovaoblast;
drop sequence seq_caj;
drop sequence seq_dodavatel;
drop sequence seq_odberatel;
drop sequence seq_varka;
drop sequence seq_objednavka;
drop sequence seq_po;

drop procedure prc_upravMnozstvo;

/***************************************************************************/
/* Vytvor tabulky, ktore budu drzat informacie.
/***************************************************************************/
create table Odberatel(
  pk INTEGER NOT NULL PRIMARY KEY,
  meno NVARCHAR2(20) NOT NULL,
  priezvisko NVARCHAR2(20) NOT NULL,
  adresaBydliska NVARCHAR2(50),
  dodaciaAdresa NVARCHAR2(50) NOT NULL,
  email VARCHAR2(50),
  telefonneCislo NUMBER(9, 0),
  odberNoviniek NUMBER(1, 0) NOT NULL check(odberNoviniek in (0, 1))
);


create table Dodavatel(
  pk INTEGER NOT NULL PRIMARY KEY,
  nazov NVARCHAR2(50) NOT NULL,
  ico NUMBER(8, 0) NOT NULL,
  weblink VARCHAR2(50),
  sidlo NVARCHAR2(50)
);


create table CajovaOblast(
  pk INTEGER NOT NULL PRIMARY KEY,
  nazov NVARCHAR2(50) NOT NULL,
  popis NVARCHAR2(500),
  typickyDruh NVARCHAR2(50),
  charaktCajov NVARCHAR2(500)
);


create table Caj(
  pk INTEGER NOT NULL PRIMARY KEY,
  nazov NVARCHAR2(50) NOT NULL,
  druh NVARCHAR2(50),
  krajinaPovodu NVARCHAR2(50),
  kvalita VARCHAR2(5),
  chut NVARCHAR2(500),
  dobaLuhovania NUMBER(2, 0),
  zdravotneUcinky NVARCHAR2(500),
  cajovaoblast_pk INTEGER,
  dodavatel_pk INTEGER NOT NULL
);


create table Varka(
  pk INTEGER NOT NULL PRIMARY KEY,
  /* Cena za 100 g*/
  cena NUMBER(5, 2) NOT NULL,
  /* V gramoch */
  dostupneMnozstvo NUMBER(8, 0),
  datumExpiracie DATE NOT NULL,
  /* V desatinnom cisle vyjadrujucom percenta */
  zlava NUMBER(3, 2) NOT NULL,
  miestoNaSklade NUMBER(2, 0),
  caj_pk INTEGER NOT NULL
);


create table Objednavka(
  pk INTEGER NOT NULL PRIMARY KEY,
  stav NVARCHAR2(20) NOT NULL,
  datumPrijatia DATE,
  stornoPoplatok NUMBER(5, 2),
  odberatel_pk INTEGER NOT NULL
);


create table PolozkaObjednavky(
  pk INTEGER NOT NULL PRIMARY KEY,
  objednavka_pk INTEGER NOT NULL,
  /* V gramoch */
  objednaneMnozstvo NUMBER(8, 0) NOT NULL,
  /* Cena za 100 g*/
  cena NUMBER(5, 2) NOT NULL,
  varka_pk INTEGER NOT NULL
);

/***************************************************************************/
/* Vytvor vztahy medzi tabulkami
/***************************************************************************/

alter table Caj add constraint fk_caj_cajovaoblast foreign key (cajovaoblast_pk) 
references CajovaOblast on delete cascade;

alter table Caj add constraint fk_caj_dodavatel foreign key (dodavatel_pk)
references Dodavatel on delete cascade;

alter table Varka add constraint fk_varka_caj foreign key (caj_pk) 
references Caj on delete cascade;

alter table Objednavka add constraint fk_objednavka_odberatel foreign key (odberatel_pk) 
references Odberatel on delete cascade;

alter table PolozkaObjednavky add constraint fk_po_objednavka foreign key (objednavka_pk) 
references Objednavka on delete cascade;

alter table PolozkaObjednavky add constraint fk_po_varka foreign key (varka_pk) 
references Varka on delete cascade;

/***************************************************************************/
/* Vytvor sekvencie pre primarne kluce
/***************************************************************************/
create sequence seq_odberatel
minvalue 0 nomaxvalue start with 1 increment by 1;

create sequence seq_dodavatel
minvalue 0 nomaxvalue start with 1 increment by 1;

create sequence seq_cajovaoblast
minvalue 0 nomaxvalue start with 1 increment by 1;

create sequence seq_caj
minvalue 0 nomaxvalue start with 1 increment by 1;

create sequence seq_varka
minvalue 0 nomaxvalue start with 1 increment by 1;

create sequence seq_objednavka
minvalue 0 nomaxvalue start with 1 increment by 1;

create sequence seq_po
minvalue 0 nomaxvalue start with 1 increment by 1;

/***************************************************************************/
/* Vytvor trigery  pre primarne kluce
/***************************************************************************/
create or replace trigger tr_odberatel
  before insert on Odberatel
  for each row
  begin
    select seq_odberatel.nextval into :new.pk from dual;
  end;
/

create or replace trigger tr_dodavatel
  before insert on Dodavatel
  for each row
  begin
    select seq_dodavatel.nextval into :new.pk from dual;
  end;
/

create or replace trigger tr_cajovaoblast
  before insert on CajovaOblast
  for each row
  begin
    select seq_cajovaoblast.nextval into :new.pk from dual;
  end;
/

create or replace trigger tr_caj
  before insert on Caj
  for each row
  begin
    select seq_caj.nextval into :new.pk from dual;
  end;
/

create or replace trigger tr_varka
  before insert on Varka
  for each row
  begin
    select seq_varka.nextval into :new.pk from dual;
  end;
/

create or replace trigger tr_objednavka
  before insert on Objednavka
  for each row
  begin
    select seq_objednavka.nextval into :new.pk from dual;
  end;
/

create or replace trigger tr_po
  before insert on PolozkaObjednavky
  for each row
  begin
    select seq_po.nextval into :new.pk from dual;
  end;
/

/***************************************************************************/
/* Procedura na upravu dostupneho mnostva pri vytvoreni / zruseni
/* polozky objednavky, spolu s trigerom ktory ju spusti.
/***************************************************************************/
CREATE OR REPLACE PROCEDURE prc_upravMnozstvo(varkaPk INTEGER, mnozstvo NUMBER) AS
    BEGIN
      UPDATE Varka
      SET dostupneMnozstvo = dostupneMnozstvo + mnozstvo
      WHERE pk = varkaPk;
    END;
/

CREATE OR REPLACE TRIGGER tr_nova_polozka 
    AFTER INSERT ON PolozkaObjednavky
    FOR EACH ROW BEGIN
        prc_upravMnozstvo(:new.varka_pk, -:new.objednaneMnozstvo);
    END;
/

CREATE OR REPLACE TRIGGER tr_zrusena_polozka 
    AFTER DELETE ON PolozkaObjednavky
    FOR EACH ROW BEGIN
        prc_upravMnozstvo(:new.varka_pk, :new.objednaneMnozstvo);
    END;
/
    
/***************************************************************************/
/* Vytvor index pre rychlejsie hladanie expirovanych varok
/***************************************************************************/
CREATE INDEX idx_expiracia ON Varka (datumExpiracie);

/***************************************************************************/
/* Napln tabulky datami
/***************************************************************************/
/*napln�n� tabulky Odberatel*/
insert into Odberatel (pk, meno, priezvisko, adresaBydliska, dodaciaAdresa, email, telefonneCislo, odberNoviniek)
values (null, 'Tom�', '��ek', 'Jablo�ov� 456 V�esina', 'Jablo�ov� 456 V�esina', 'xcizek12@stud.fit.vutbr.cz', 123456789, 1);
insert into Odberatel (pk, meno, priezvisko, adresaBydliska, dodaciaAdresa, email, telefonneCislo, odberNoviniek) 
values (null, 'Ivan', '�ev��k', null, 'Halalovka 10 Tren��n', 'xsevci50@stud.fit.vutbr.cz', 987654321, 0);
insert into Odberatel (pk, meno, priezvisko, adresaBydliska, dodaciaAdresa, email, telefonneCislo, odberNoviniek) 
values (null, 'Pavol', 'Nicotka', null, 'V�de�sk� 3 Brno', 'nicotka@email.cz', 655352333, 1);

/*napln�n� tabulky Dodavatel*/
insert into Dodavatel (pk, nazov, ico, weblink, sidlo) 
values (null, 'True Tea', 12345678, 'www.truetea.com', 'Praha');
insert into Dodavatel (pk, nazov, ico, weblink, sidlo) 
values (null, 'Biogena', 54321323, 'www.biogena.com', '�esk� Bud�jovice');
insert into Dodavatel (pk, nazov, ico, weblink, sidlo) 
values (null, 'MEGAFYT-R s.r.o.', 10000000, 'www.megafyt-pharma.cz', 'Vran� nad Vltavou');
insert into Dodavatel (pk, nazov, ico, weblink, sidlo) 
values (null, 'OXALIS', 23232323, 'www.oxalis.cz', 'Slu�ovice');
insert into Dodavatel (pk, nazov, ico, weblink, sidlo) 
values (null, 'Orient Tea and Commodities Co., Ltd.', 11111111, 'orientea.en.alibaba.com', 'Hangzhou');
insert into Dodavatel (pk, nazov, ico, weblink, sidlo) 
values (null, 'Changsha Organic Herb Inc.', 22222222, 'www.organic-herb.com', 'Changsha');

/*napln�n� tabulky �ajov�ch oblast�*/
insert into CajovaOblast (pk, nazov, popis, typickyDruh, charaktCajov) 
values (null, 'Rous�nov', 'V t�to oblasti je velmi tepl� a vlhk� podneb�.', 'Zelen� �aj', '�aje z t�to oblasti jsou kysel�.');
insert into CajovaOblast (pk, nazov, popis, typickyDruh, charaktCajov) 
values (null, 'Jiangnan', 'V oblasti je dostatok slunka a vlahy pro p�stov�n� t�ch nejlep��ch �aj�.', '�ern� �aj', '�aje z t�to oblasti jsou intenzivn�.');
insert into CajovaOblast (pk, nazov, popis, typickyDruh, charaktCajov) 
values (null, 'V�esina', 'V t�to oblasti je velmi tepl� a such� podneb�.', '�ern� �aj', '�aje z t�to oblasti jsou ho�k�.');
insert into CajovaOblast (pk, nazov, popis, typickyDruh, charaktCajov) 
values (null, 'Rio Grande do Sul', 'V oblasti je vlhk� subtropick� podneb�.', 'mate', '�aje z t�to oblasti jsou v�razn� a ovocn�.');
insert into CajovaOblast (pk, nazov, popis, typickyDruh, charaktCajov) 
values (null, 'Fujian', 'V oblasti p�eva�uje vlhk� ale tepl� podneb�.', '�ern� �aj', '�aje z t�to oblasti jsou jemn�.');
insert into CajovaOblast (pk, nazov, popis, typickyDruh, charaktCajov) 
values (null, 'Zheijang', 'Velmi slune�n� a tepl� oblast.', 'Zelen� �aj', '�aje z t�to oblasti jsou intenzivn�.');

/*napln�n� tabulky �aj�*/
insert into Caj (pk, nazov, druh, krajinaPovodu, kvalita, chut, dobaluhovania, zdravotneucinky, cajovaoblast_pk, dodavatel_pk) 
values (null, 'Dra�� dech', 'Zelen�', '�esko', 'OP', 'Osv�uj�ci kyselkav�', 5, null, 1, 1);
insert into Caj (pk, nazov, druh, krajinaPovodu, kvalita, chut, dobaluhovania, zdravotneucinky, cajovaoblast_pk, dodavatel_pk) 
values (null, 'Irish cream', '�ern�', '��na', 'TGFOP', 'V�razn� s p��chut� karamelu', 3, null, 2, 2);
insert into Caj (pk, nazov, druh, krajinaPovodu, kvalita, chut, dobaluhovania, zdravotneucinky, cajovaoblast_pk, dodavatel_pk) 
values (null, '�ep�kov� �aj', 'Bilinn�', '�esko', null, 'Trpk�', 8, 'Zastavuje pr�jem', null, 3);
insert into Caj (pk, nazov, druh, krajinaPovodu, kvalita, chut, dobaluhovania, zdravotneucinky, cajovaoblast_pk, dodavatel_pk) 
values (null, 'Mate IQ', 'Mate', 'Braz�lia', null, 'Ovocno kvetov�', 5, 'Povzbudzuje organizmus', 4, 4);
insert into Caj (pk, nazov, druh, krajinaPovodu, kvalita, chut, dobaluhovania, zdravotneucinky, cajovaoblast_pk, dodavatel_pk) 
values (null, 'Fujian organick�', 'Oolong', '��na', 'FOP', 'Bohat� jemne hork�', 4, null, 5, 5);
insert into Caj (pk, nazov, druh, krajinaPovodu, kvalita, chut, dobaluhovania, zdravotneucinky, cajovaoblast_pk, dodavatel_pk) 
values (null, 'Tian Mu Yun Wu', 'Zelen�', '��na', 'OP', 'Jemn� osvie�uj�ca', 3, null, 6, 5);
insert into Caj (pk, nazov, druh, krajinaPovodu, kvalita, chut, dobaluhovania, zdravotneucinky, cajovaoblast_pk, dodavatel_pk) 
values (null, 'Gunpowder', 'Zelen�', '��na', 'OP', 'Zemit�', 4, 'Siln� antioxidant', 6, 5);

/*napln�n� tabulky v�rek*/
insert into Varka (pk, cena, dostupneMnozstvo, datumExpiracie, zlava, miestoNaSklade, caj_pk) 
values (null, 80, 12000, to_date('20180401', 'YYYYMMDD'), 0, 5, 1);
insert into Varka (pk, cena, dostupneMnozstvo, datumExpiracie, zlava, miestoNaSklade, caj_pk) 
values (null, 93, 1753, to_date('20150515', 'YYYYMMDD'), 0.05, 6, 2);
insert into Varka (pk, cena, dostupneMnozstvo, datumExpiracie, zlava, miestoNaSklade, caj_pk) 
values (null, 68, 1347, to_date('20181123', 'YYYYMMDD'), 0, 1, 3);
insert into Varka (pk, cena, dostupneMnozstvo, datumExpiracie, zlava, miestoNaSklade, caj_pk) 
values (null, 71, 9890, to_date('20190722', 'YYYYMMDD'), 0, 2, 4);
insert into Varka (pk, cena, dostupneMnozstvo, datumExpiracie, zlava, miestoNaSklade, caj_pk) 
values (null, 86, 3571, to_date('20180314', 'YYYYMMDD'), 0, 3, 5);
insert into Varka (pk, cena, dostupneMnozstvo, datumExpiracie, zlava, miestoNaSklade, caj_pk) 
values (null, 102, 8703, to_date('20160703', 'YYYYMMDD'), 0.05, 4, 6);
insert into Varka (pk, cena, dostupneMnozstvo, datumExpiracie, zlava, miestoNaSklade, caj_pk) 
values (null, 88, 6039, to_date('20150901', 'YYYYMMDD'), 0.1, 5, 6);

/*napln�n� tabulky objedn�vek*/
insert into Objednavka (pk, stav, datumPrijatia, stornoPoplatok, odberatel_pk) 
values (null, 'p�ijata', to_date('20130320', 'YYYYMMDD'), 200, 1);
insert into Objednavka (pk, stav, datumPrijatia, stornoPoplatok, odberatel_pk) 
values (null, '�ek� na p�ijet�', to_date('20130325', 'YYYYMMDD'), 343, 2);
insert into Objednavka (pk, stav, datumPrijatia, stornoPoplatok, odberatel_pk) 
values (null, 'neodesl�na', null, null, 2);

/*napln�n� tabulky polo�ek objedn�vky*/
insert into PolozkaObjednavky (pk, objednavka_pk, objednaneMnozstvo, cena, varka_pk) 
values (null, 1, 800, 80, 1);
insert into PolozkaObjednavky (pk, objednavka_pk, objednaneMnozstvo, cena, varka_pk) 
values (null, 1, 300, 90, 2);
insert into PolozkaObjednavky (pk, objednavka_pk, objednaneMnozstvo, cena, varka_pk) 
values (null, 2, 500, 93, 2);
insert into PolozkaObjednavky (pk, objednavka_pk, objednaneMnozstvo, cena, varka_pk) 
values (null, 1, 1130, 71, 4);
insert into PolozkaObjednavky (pk, objednavka_pk, objednaneMnozstvo, cena, varka_pk) 
values (null, 2, 560, 86, 5);
insert into PolozkaObjednavky (pk, objednavka_pk, objednaneMnozstvo, cena, varka_pk) 
values (null, 2, 400, 79, 7);

/***************************************************************************/
/* Sprav zakladny nahlad na tabulky
/***************************************************************************/
select * from Odberatel;
select * from Dodavatel;
select * from CajovaOblast;
select * from Caj;
select * from Varka;
select * from Objednavka;
select * from PolozkaObjednavky;

/***************************************************************************/
/* Pridaj prava druhemu uzivatelovi
/***************************************************************************/
GRANT ALL PRIVILEGES ON Objednavka TO xsevci50;
GRANT ALL PRIVILEGES ON Odberatel TO xsevci50;
GRANT ALL PRIVILEGES ON Varka TO xsevci50;
GRANT ALL PRIVILEGES ON Caj TO xsevci50;
GRANT ALL PRIVILEGES ON CajovaOblast TO xsevci50;
GRANT ALL PRIVILEGES ON Dodavatel TO xsevci50;
GRANT ALL PRIVILEGES ON PolozkaObjednavky TO xsevci50;

GRANT ALL PRIVILEGES ON seq_cajovaoblast TO xsevci50;
GRANT ALL PRIVILEGES ON seq_caj TO xsevci50;
GRANT ALL PRIVILEGES ON seq_dodavatel TO xsevci50;
GRANT ALL PRIVILEGES ON seq_odberatel TO xsevci50;
GRANT ALL PRIVILEGES ON seq_varka TO xsevci50;
GRANT ALL PRIVILEGES ON seq_objednavka TO xsevci50;
GRANT ALL PRIVILEGES ON seq_po TO xsevci50;

/***************************************************************************/
/* Uloz zmeny
/***************************************************************************/
COMMIT;