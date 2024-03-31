const char HELP[] =
"\
NAME\n\
\tvsteg ver 0.1\n\n\
SYNOPSYS\n\
\tvsteg <option> <video file> <secret file> <password>\n\
\tvsteg -h\n\n\
DESCRIPTION\n\
\tvsteg allows to encapsulate a secret file into a video file and extract it back.\n\
\tVideo file continues to play normally after encapsulation.\n\
\tAfter extraction, we have two source files - video file and secret file.\n\
\tThis is one of the ways of steganography.\n\
\tHeader and the end of the secret file file will be encrypted\n\
\tto protect file against it type identification.\n\n\
OPTIONS\n\
\t-e\tEncapsulates secret file into video file.\n\
\t\tAttention! Secret file will be REMOVED!\n\
\t\tPlease, make copy if you need before using.\n\n\
\t-s\tSplits video file and secret file.\n\n\
PARAMETRS\n\
\tvideo file\tpath to video file.\n\n\
\tsecret file\tpath to secret file.\n\n\
\tpassword\tpassword to protect against unauthorized extraction of a secret file,\n\
\t\t\theader and footer encryption. Password MUST be 8 or 16 symbols.\n\
\t\t\tAttention! If you LOST the PASSWORD, FILE EXTRACTION will be IMPOSSIBLE!\n\n\
SUPPORT\n\
\thttps://t.me/Alex_Yuschenko\n\
\thttps://vsyst.ru\n\
";

const char PROMPT[] = "Use -h to see help.";
