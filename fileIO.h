#ifndef FILEIO_H
#define FILEIO_H


int CxToRxConvert(editorRow *row, int cx);
void editorOpen(char *filename);
void editorWriteChar(int c);
void editorSave();

#endif