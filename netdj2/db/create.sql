DROP TABLE IF EXISTS Songs;
DROP TABLE IF EXISTS Covers;

CREATE TABLE Covers (
  id bigint(20) NOT NULL auto_increment,
  filename varchar(250) NOT NULL,

  PRIMARY KEY (id)
);

CREATE TABLE Songs (
  id bigint(20) NOT NULL auto_increment,
  artist char(100) NOT NULL,
  album char(100) default NULL,
  genre char(20) NOT NULL,
  track int(3) default NULL,
  year int(4) default NULL,
  size bigint(6) NOT NULL,
  owner char(40) NOT NULL,
  audiotype int(2) NOT NULL,
  comment char(150) default NULL,
  date timestamp(14) NOT NULL,
  filename char(255) NOT NULL,
  cover bigint(20) default NULL,

  PRIMARY KEY  (id),
  CONSTRAINT fk_cover FOREIGN KEY (cover) REFERENCES Covers(id)
);
