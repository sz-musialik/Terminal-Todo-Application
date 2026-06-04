CREATE TABLE Todos (
	Id INTEGER PRIMARY KEY NOT NULL,
	Title VARCHAR(128) NOT NULL,
	Completed INTEGER NOT NULL
);

INSERT INTO Todos values(NULL, 'Programm this application', 0);
INSERT INTO Todos values(NULL, 'Make dinner', 0);
