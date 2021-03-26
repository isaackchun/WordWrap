Word Wrapper

C program that takes texts from file or stdin and reformats it to fit a certain number of colums that user inputs

To run with file/directory:

./ww (column number) (file name)


To run with stdin:

./ww (column number)


The program also takes directory and wraps all the file inside directory and create file named "wrap.filename" for each file. But it will ignore file name that starts with "wrap."
If its not directory mode, it will output on stdout.