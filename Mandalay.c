/*
 * Mandalay for Windows & Linux
 * by Peter Neubauer <peterneubauer2@gmail.com>
 * with refinements by Simon Naarmann
 *
 * History:
 *   2008-07-07:
 *     - This is Version 1.1.
 *     - Added support for configuration file.
 *     - Introduced header information and History.
 *   prior date:
 *     - Version 1.0
 *     - No change history
 *
 * You do not have any explicit or implicit permission to copy, view, alter,
 * move, transmit, compile or otherwise use this file or the information
 * (code etc.) contained within. Contact the author separately to obtain a
 * software license for this software.
 */

#include <allegro.h>

#ifdef _WIN32
#include <winalleg.h>
#include <winsock2.h>
#else
#include <netdb.h>
#endif

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
    WSADATA WSAData;
    SOCKET sck=INVALID_SOCKET;
    SOCKADDR_IN sAdresse;
#else
    int sck;
#endif

const char* cfg_filename = "settings.txt";

/* Main game vars */
int tower[12][5];
int tower_count = 12;

/* General app vars */
char cfgline[80]; /* Line from settings file, see getcfgline */
int gameover;
int client;
int pcol[2]; /* Player colors get randomized in init_globals */
int draw_cursor;
int cursor;
char infotxt [256];
double lastmove_from; /* May be between 2 towers if it doesn't exist anymore */
int lastmove_to;
union { int number; char str [4]; } netbuf; /* for RNG, dictated by server */

/* Config vars (can be changed by config) */
int CONFIG_PORT;
int CONFIG_RES_X;
int CONFIG_RES_Y;
int CONFIG_FULLSCREEN;
int CONFIG_BIGCIRCLE_RADIUS;
int CONFIG_BIGCIRCLE_YCORRECTION;
int CONFIG_LASTMOVE_COLOR_R;
int CONFIG_LASTMOVE_COLOR_G;
int CONFIG_LASTMOVE_COLOR_B;
int CONFIG_ERROR_COLOR_R;
int CONFIG_ERROR_COLOR_G;
int CONFIG_ERROR_COLOR_B;
int CONFIG_LASTMOVE_RADIUS ;
int CONFIG_SMALLCIRCLE_RADIUS;
int CONFIG_CIRCLEUP_DISTANCE;
int CONFIG_MOVEARROW_LEFT_R;
int CONFIG_MOVEARROW_LEFT_G;
int CONFIG_MOVEARROW_LEFT_B;
int CONFIG_MOVEARROW_RIGHT_R;
int CONFIG_MOVEARROW_RIGHT_G;
int CONFIG_MOVEARROW_RIGHT_B;

#define MOVEARROW_LEFT  makecol (CONFIG_MOVEARROW_LEFT_R , CONFIG_MOVEARROW_LEFT_G , CONFIG_MOVEARROW_LEFT_B )
#define MOVEARROW_RIGHT makecol (CONFIG_MOVEARROW_RIGHT_R, CONFIG_MOVEARROW_RIGHT_G, CONFIG_MOVEARROW_RIGHT_B)
#define LASTMOVE_COLOR  makecol (CONFIG_LASTMOVE_COLOR_R , CONFIG_LASTMOVE_COLOR_G , CONFIG_LASTMOVE_COLOR_B )
#define ERROR_COLOR     makecol (CONFIG_ERROR_COLOR_R    , CONFIG_ERROR_COLOR_G    , CONFIG_ERROR_COLOR_B    )

void default_config (); /* Set all default config values */
int read_config (); /* read config from file cfg_filename */
int getcfgline (FILE* file);
int getrand ();
void init_globals ();
void get_turn (int player);
void delete_tower (int t);
int  has_valid_moves (int player);
int  is_valid_move (int t, int dir);
int  process_turn (int t, int dir);

int towerheight (int t);
int towerx (double t);
int towery (double t);
int towerxplush (double t);
int toweryplush (double t);
void info ();
void draw_arrow (int fromx, int fromy, int tox, int toy, int color);
void redraw ();

void cursor_goto_prev ();
void cursor_goto_next ();

int main(int argc, char* argv[])
{
	int cfgresult = read_config ();

	int wndmode = CONFIG_FULLSCREEN ? GFX_AUTODETECT_FULLSCREEN : GFX_AUTODETECT_WINDOWED;

	allegro_init();
	install_keyboard();
	set_color_depth (desktop_color_depth());
	set_gfx_mode(wndmode, CONFIG_RES_X, CONFIG_RES_Y, 0, 0);

	if (cfgresult == 1) {
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Could not find \"%s\". Using default settings.", cfg_filename);
        readkey();
		clear_bitmap(screen);
	} else
	if (cfgresult == 2) {
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "A line in \"%s\" is too long. Using default settings.", cfg_filename);
        readkey();
		clear_bitmap(screen);
	} else
	if (cfgresult == 3) {
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "\"%s\" contains an invalid config option. Using default settings.", cfg_filename);
        readkey();
		clear_bitmap(screen);
	}

    #ifdef _WIN32
    if(WSAStartup(MAKEWORD(1,1),&WSAData)!=0)
    {
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "WSAStartup failed. Error: %d", WSAGetLastError());
        readkey();
	exit (0);
    }
    #endif

	/* setup network stuff */
	if (argc > 1) {
		/* Client */
		client = 1;

        #ifdef _WIN32
        sck=socket(AF_INET,SOCK_STREAM,0);
if(sck==INVALID_SOCKET)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Allocating socket failed. Error: %d", WSAGetLastError());
        readkey();
	exit (0);
}
sAdresse.sin_family=AF_INET;
sAdresse.sin_addr.s_addr=inet_addr(argv[1]);
sAdresse.sin_port=htons(CONFIG_PORT);
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Connecting to %s...", argv[1]);
		                     int serverlen=sizeof(sAdresse);
int Connection=connect(sck,(struct sockaddr*)&sAdresse,serverlen); if(Connection==-1)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Connect Error: %d", WSAGetLastError());
        readkey();
	exit (0);
}
        #else
		struct sockaddr_in serv_addr;
		struct hostent *server;
		sck = socket(AF_INET, SOCK_STREAM, 0);
if(sck < 0)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Allocating socket failed. Error: %d", errno);
        readkey();
	exit (0);
}
		server = gethostbyname(argv[1]);
if(server == NULL)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Could not find host \"%s\". Error: %d", argv[1], h_errno);
        readkey();
	exit (0);
}
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr,
		     (char *)&serv_addr.sin_addr.s_addr,
		     server->h_length);
		serv_addr.sin_port = htons(CONFIG_PORT);
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Connecting to %s...", argv[1]);
		int con = connect(sck,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
if(con < 0)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Connect Error: %d", errno);
        readkey();
	exit (0);
}
		#endif
	} else {
		/* Server */
		client = 0;

        #ifdef _WIN32
        sck=socket(AF_INET,SOCK_STREAM,0);
if(sck==INVALID_SOCKET)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Allocating socket failed. Error: %d", WSAGetLastError());
        readkey();
	exit (0);
}
sAdresse.sin_family=AF_INET;
sAdresse.sin_addr.s_addr=htonl(INADDR_ANY);
sAdresse.sin_port=htons(CONFIG_PORT);
int serverlen=sizeof(sAdresse);
int Binden = bind(sck,(struct sockaddr*)&sAdresse,serverlen);
if(Binden==-1)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Bind Error: %d", WSAGetLastError());
        readkey();
	exit (0);
} int Listen=listen(sck,5);
if(Listen==-1)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Listen Error: %d", WSAGetLastError());
        readkey();
	exit (0);
}
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Waiting for client on port %d...", CONFIG_PORT);
int clientlen=sizeof(sAdresse);
sck=
accept(sck,(struct sockaddr*)&sAdresse,&clientlen);
if(sck==-1)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Accept Error: %d", WSAGetLastError());
        readkey();
	exit (0);
}
        #else

		int listener = socket(AF_INET, SOCK_STREAM, 0);
if(listener < 0)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Allocating socket failed. Error: %d", errno);
        readkey();
	exit (0);
}
    	struct sockaddr_in serv_addr, cli_addr;
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(CONFIG_PORT);
		int b = bind(listener, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
if(b < 0)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Bind Error: %d", errno);
        readkey();
	exit (0);
}
		int l = listen(listener, 1);
if(l < 0)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Listen Error: %d", errno);
        readkey();
	exit (0);
}
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Waiting for client on port %d...", CONFIG_PORT);
		int clilen = sizeof(cli_addr);
		sck = accept(listener, (struct sockaddr *) &cli_addr, &clilen);
if(sck < 1)
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Accept Error: %d", errno);
        readkey();
	exit (0);
}
		#endif
		srand(time(0));
	}

	init_globals ();

	int current_player = getrand() % 2;
	while (!gameover)
	{
		cursor = 0; cursor_goto_next(); cursor_goto_prev();
		redraw ();
		get_turn (current_player);
		current_player = (int)!current_player;
	}

	textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2,
	                     ERROR_COLOR, 0, "Player %d wins!", current_player+1);

#ifdef _WIN32
closesocket(sck);
#endif

	readkey();
	return 0;
}
END_OF_MAIN();

int getrand ()
{
	if (client) {
        int r = recv(sck, netbuf.str, 4 * sizeof(char), 0); /* get random nr from server */
if(r != 4 * sizeof(char))
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Network Error on Receive: %d, %d", r, errno);
        readkey();
	exit (0);
}
	} else {
		netbuf.number = htonl(rand());
        int s = send(sck, netbuf.str, 4 * sizeof(char), 0); /* send random nr to client */
if(s != 4 * sizeof(char))
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Network Error on Send: %d, %d", s, errno);
        readkey();
	exit (0);
}
	}
	return ntohl(netbuf.number);
}

void init_globals ()
{
	gameover = 0;
	tower_count = 12;

    int lower[] = { tower_count/2, tower_count/2 };
    int upper[] = { tower_count/2, tower_count/2 };

	/* init towers
	   -1 = no owner
       0 = owned by server
       1 = owned by client
	*/
	int i, j; /* current tower, current height */
	for (i = 0; i < tower_count; ++i) {
        if ((lower[0] > 0) && (lower[1] > 0)) {
            int owner = getrand() % 2; tower[i][0] = owner; lower[owner]--;
        } else {
            tower[i][0] = (lower[0] == 0) ? 1 : 0;
        }
        if ((upper[0] > 0) && (upper[1] > 0)) {
            int owner = getrand() % 2; tower[i][1] = owner; upper[owner]--;
        } else {
            tower[i][1] = (upper[0] == 0) ? 1 : 0;
        }
		for (j = 2; j < 5; ++j) tower[i][j] = -1;
	}

    int dark = getrand() % 2; /* who gets the darker color? */
    {  /* now calculate 2 colors with a min. distance */
        int r1, g1, b1;
        int r2, g2, b2;
        int distance = 30 * 30 * 30;

        do {
            r1 = getrand()%128 + 128;
            g1 = getrand()%128 + 128;
            b1 = getrand()%128 + 128;
            r2 = getrand()%128 + 128;
            g2 = getrand()%128 + 128;
            b2 = getrand()%128 + 128;
        } while ( (r1-r2)*(r1-r2)
                + (g1-g2)*(g1-g2)
                + (b1-b2)*(b1-b2) < (distance -= 80) );

        pcol[ dark] = makecol(r1, g1, b1);
        pcol[!dark] = makecol(r2, g2, b2);
    }

    draw_cursor = FALSE;
    lastmove_from = -1;
    lastmove_to = -1;
}

int towerheight (int t)
{
    int h;
    for (h = 1; h < 5 && tower[t][h] != -1; ++h);
    return h;
}

int towerx (double t)
{
    double a = t / tower_count * 2 * M_PI;
    return SCREEN_W/2 + cos(a) * CONFIG_BIGCIRCLE_RADIUS;
}

int towery (double t)
{
    double a = t / tower_count * 2 * M_PI;
    return SCREEN_H/2 + sin(a) * CONFIG_BIGCIRCLE_RADIUS + CONFIG_BIGCIRCLE_YCORRECTION;
}

/* towery plus height */
int toweryplush (double t)
{
    double a = t / tower_count * 2 * M_PI;
    return SCREEN_H/2 + sin(a) * CONFIG_BIGCIRCLE_RADIUS +
           CONFIG_BIGCIRCLE_YCORRECTION - (towerheight(t)-1) * CONFIG_CIRCLEUP_DISTANCE;
}

void info (char* txt)
{
	strcpy (infotxt, txt);
	redraw();
}

void draw_arrow (int fromx, int fromy, int tox, int toy, int color)
{
    int lenx = tox - fromx;
    int leny = toy - fromy;
    int startx = fromx + lenx * 1 / 7;
    int starty = fromy + leny * 1 / 7;
    int endx = fromx + lenx * 6 / 7;
    int endy = fromy + leny * 6 / 7;
    int side1x = fromx + lenx * 4/7 + leny * 1/7;
    int side1y = fromy + leny * 4/7 - lenx * 1/7;
    int side2x = fromx + lenx * 4/7 - leny * 1/7;
    int side2y = fromy + leny * 4/7 + lenx * 1/7;
    line (screen, startx, starty, endx, endy, color);
    line (screen, endx, endy, side1x, side1y, color);
    line (screen, endx, endy, side2x, side2y, color);
}

void redraw ()
{
	acquire_screen ();

	clear_bitmap(screen);
	textprintf_centre_ex(screen, font, SCREEN_W/2, 10, makecol(255, 255, 255), 0,
	                     "You are player %d.", (int)client+1);
	textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H - 20, makecol(255, 255, 255), 0, "%s", infotxt);

	int t, i; /* current tower, current height */

	/* draw towers */
	for (t = 0; t < tower_count; ++t) {
		/* find circle position for current tower */
		int tower_x = towerx(t);
		int tower_y = towery(t);

		/* draw all needed circles */
		for (i = 0; i < 5; ++i) {
			int color;
			if (tower[t][i] == -1) break;
			if (tower[t][i] == 0) color = pcol[0]; /* makecol (230, 230, 230); */
			else                  color = pcol[1]; /* makecol (70 , 50 , 230); */
			
            tower_y -= CONFIG_CIRCLEUP_DISTANCE;
            circlefill(screen, tower_x, tower_y, CONFIG_SMALLCIRCLE_RADIUS, color);
            circle    (screen, tower_x, tower_y, CONFIG_SMALLCIRCLE_RADIUS, 0);
            /* Hoehe schreiben */
            textprintf_centre_ex(screen, font, tower_x, tower_y - 4, 0, -1, "%d", i+1);
		}

		if ((lastmove_to != -1) && (t == lastmove_to)) {
		    circlefill (screen, tower_x, tower_y, CONFIG_LASTMOVE_RADIUS, LASTMOVE_COLOR);
		    draw_arrow (towerx(lastmove_from), toweryplush(lastmove_from),
                        towerx(lastmove_to)  , toweryplush(lastmove_to)  , LASTMOVE_COLOR);
		}
	}

    if (draw_cursor) {
        int tcurx = towerx(cursor);
        int tcury = toweryplush(cursor);
        int tleftx = towerx(cursor - towerheight(cursor));
        int tlefty = toweryplush(cursor - towerheight(cursor));
        int trightx = towerx(cursor + towerheight(cursor));
        int trighty = toweryplush(cursor + towerheight(cursor));
        if (is_valid_move (cursor, -1)) draw_arrow (tcurx, tcury, tleftx, tlefty, MOVEARROW_LEFT);
        if (is_valid_move (cursor,  1)) draw_arrow (tcurx, tcury, trightx, trighty, MOVEARROW_RIGHT);
    }

	release_screen ();
}

void get_turn (int player)
{
	if (!has_valid_moves(player)) { gameover = 1; return; }

	if (player == client) {
	    draw_cursor = TRUE;
		/* get move input from GUI */
		/* first step: cursor selects source tower */
		int key;
		int dir;
		startinput:
		clear_keybuf();
		for (;;) {
			info("Use [LEFT] and [RIGHT] to choose a tower and [SPACE] to select it.");
			key = readkey() >> 8;
			if (key == KEY_LEFT)  cursor_goto_prev();
			if (key == KEY_RIGHT) cursor_goto_next();
			if (key == KEY_SPACE) break;
            if (key == KEY_ESC) exit(0);
		}
		/* 2nd step: select direction to move in */
		for (;;) {
			info("Use [LEFT] and [RIGHT] to choose a direction, [SPACE] to abort.");
			key = readkey() >> 8;
			if (key == KEY_LEFT ) { dir = -1; break; }
			if (key == KEY_RIGHT) { dir =  1; break; }
			if (key == KEY_SPACE) goto startinput;
            if (key == KEY_ESC) exit(0);
		}
		/* 3rd step: validate and process */
		if (!process_turn (cursor, dir)) {
			textout_centre_ex(screen, font, "Invalid Move!", SCREEN_W/2, SCREEN_H/2, makecol(255, 0, 0), 0);
			key = readkey();
            if (key == KEY_ESC) exit(0);
			get_turn (player);
		}
		else {
			/* send move over network */
			char buf[2] = { cursor, dir };
			int s = send(sck, buf, 2 * sizeof(char), 0);
if(s != 2 * sizeof(char))
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Network Error on Send: %d, %d", s, errno);
        readkey();
	exit (0);
}
			draw_cursor = FALSE;
		}
	}
	else {
		/* get move input from network */
		char buf[2];
		info ("Waiting for opponent move...");
		int r = recv(sck, buf, 2 * sizeof(char), 0);
if(r != 2 * sizeof(char))
{
		textprintf_centre_ex(screen, font, SCREEN_W/2, SCREEN_H/2, ERROR_COLOR, 0,
		                     "Network Error on Receive: %d, %d", r, errno);
        readkey();
	exit (0);
}
		process_turn (buf[0], buf[1]);
	}
}

int has_valid_moves (int player)
{
	int c = cursor;
	do {
		int h;
		for (h = 4; tower[c][h] == -1; --h);
		if (tower[c][h] == player)
			if (is_valid_move(c, -1) || is_valid_move(c, 1)) return 1;
		c++; if (c < 0) c = tower_count-1; if (c >= tower_count) c = 0;
	} while (c != cursor);

	return 0;
}

int is_valid_move (int t, int dir)
{
	/* sanity checks */
	if ((t < 0) || (t >= tower_count)) return 0;
	if ((dir != -1) && (dir != 1))     return 0;

	/* now find out who's on top of that tower */
	int height;
	for (height = 0; height < 5; ++height) if (tower[t][height] == -1) break;

	int i; /* how far we go from tower t */
	for (i = dir; i != (height+1) * dir; i += dir) {
		int examine = (t + i + tower_count) % tower_count;
		if (tower[examine][4] != -1) return 0; /* you shall not pass */
	}

	return 1;
}

/* Process turn = validate, move tower data around and generally apply turn.
  t = tower to move from, dir = -1 or 1, direction
Return 1 on success, 0 on invalid turn */
int process_turn (int t, int dir)
{
	if (!is_valid_move(t, dir)) return 0;

	/* now find out who's on top of that tower */
	int height = towerheight(t);
	int i;

    if (height > 1) lastmove_from = t;
    else lastmove_from = t - 0.5;

	int newtower = (t + height * dir + tower_count) % tower_count;
	lastmove_to = newtower;
	if ((height == 1) && (newtower > t)) lastmove_to--;

	/* find target tower height and do the changes */
	for (i = 0; i < 5; ++i) {
		if (tower[newtower][i] == -1) {
			tower[newtower][i] = tower[t][height-1];
			tower[t][height-1] = -1;
			if (height == 1) delete_tower(t);
			return 1;
		}
	}

	/* how did we get here? */
	return 0;
}

void delete_tower (int t)
{
	for (; t < tower_count -1; ++t) {
		int h;
		for (h = 0; h < 5; ++h) tower[t][h] = tower[t+1][h];
	}
	tower_count--;
}

void cursor_goto_prev ()
{
    int offset;
	for (offset = tower_count-1; offset > 0; --offset) {
	    int c = (cursor + offset) % tower_count;
		int h = towerheight(c) - 1;
		if ((tower[c][h] == client) && (is_valid_move(c, -1) || is_valid_move(c, 1))) {
		    cursor = c; return;
        }
	}
}

void cursor_goto_next ()
{
    int offset;
	for (offset = 1; offset < tower_count; ++offset) {
	    int c = (cursor + offset) % tower_count;
		int h = towerheight(c) - 1;
		if ((tower[c][h] == client) && (is_valid_move(c, -1) || is_valid_move(c, 1))) {
		    cursor = c; return;
        }
	}
}

/*
 * Writes next line from file to cfgline variable.
 * Returns 0 on normal success
 * Returns 1 on success + EOF
 * Returns 2 on EOF without new line data
 * Returns 3 when line is too long
 */
int getcfgline (FILE* file)
{
	int i; /* index */
	char c; /* character */
	
	for (i = 0; ; ++i) {
		c = fgetc (file);
		if (c == EOF) {
			cfgline[i] = '\0'; /* End of line -> cap string */
			return i == 0 ? 2 : 1;
		} else
		if ((c == '\n') || (c == '\r')) {
			if (i == 0) { /* Ignore empty lines, get next real line */
				i--;
				continue;
			} else {
				cfgline[i] = '\0'; /* End of line -> cap string */
				return 0;
			}
		} else
		if (i >= 79) {
			return 3; /* line too long, stop trying to read file */
		} else {
			cfgline[i] = c;
		}
	}
}

void default_config ()
{
	fprintf (stdout, "Loading default config.\n");
	CONFIG_PORT                  = 2384;
	CONFIG_RES_X                 = 640 ;
	CONFIG_RES_Y                 = 480 ;
	CONFIG_FULLSCREEN            = 0   ;
	CONFIG_BIGCIRCLE_RADIUS      = 180 ;
	CONFIG_BIGCIRCLE_YCORRECTION = 20  ;
	CONFIG_LASTMOVE_COLOR_R      = 255 ;
	CONFIG_LASTMOVE_COLOR_G      = 190 ;
	CONFIG_LASTMOVE_COLOR_B      = 30  ;
	CONFIG_ERROR_COLOR_R         = 255 ;
	CONFIG_ERROR_COLOR_G         = 123 ;
	CONFIG_ERROR_COLOR_B         = 23  ;
	CONFIG_LASTMOVE_RADIUS       = 10  ;
	CONFIG_SMALLCIRCLE_RADIUS    = 20  ;
	CONFIG_CIRCLEUP_DISTANCE     = 5   ;
    CONFIG_MOVEARROW_LEFT_R      = 120 ;
    CONFIG_MOVEARROW_LEFT_G      = 80  ;
    CONFIG_MOVEARROW_LEFT_B      = 80  ;
    CONFIG_MOVEARROW_RIGHT_R     = 80  ;
    CONFIG_MOVEARROW_RIGHT_G     = 120 ;
    CONFIG_MOVEARROW_RIGHT_B     = 80  ;
}

/*
 * Reads the config file cfg_filename
 * Returns 0 on success
 * Returns 1 on "file not found"
 * Returns 2 on "line in file too long"
 * Returns 3 on "invalid config option in file"
 */ 
int read_config ()
{
	default_config ();
		
	/* Read settings */
	FILE *fp;
	int ccount; /* set to 0 if cfg file not readable */
	
	if ( (fp = fopen (cfg_filename, "r")) == NULL) {
		fprintf (stderr, "Could not find config file \"%s\".\n", cfg_filename);
		return 1;
	} else {
	
		do {
			ccount = getcfgline (fp);
			
			if (ccount == 2) break; else
			if (ccount == 3) {
				default_config ();
				fclose (fp);
				return 2;
			}
		     
			else if (cfgline[0] == '#') continue; /* Comment */
			
			else if (strncmp(cfgline, "port "                 ,  5) == 0) CONFIG_PORT                  = atoi ( cfgline +  5 );
			else if (strncmp(cfgline, "res_x "                ,  6) == 0) CONFIG_RES_X                 = atoi ( cfgline +  6 ); 
			else if (strncmp(cfgline, "res_y "                ,  6) == 0) CONFIG_RES_Y                 = atoi ( cfgline +  6 ); 
			else if (strncmp(cfgline, "fullscreen "           , 11) == 0) CONFIG_FULLSCREEN            = atoi ( cfgline + 11 ); 
			else if (strncmp(cfgline, "bigcircle_radius "     , 17) == 0) CONFIG_BIGCIRCLE_RADIUS      = atoi ( cfgline + 17 ); 
			else if (strncmp(cfgline, "bigcircle_ycorrection ", 22) == 0) CONFIG_BIGCIRCLE_YCORRECTION = atoi ( cfgline + 22 ); 
			else if (strncmp(cfgline, "lastmove_color_r "     , 17) == 0) CONFIG_LASTMOVE_COLOR_R      = atoi ( cfgline + 17 ); 
			else if (strncmp(cfgline, "lastmove_color_g "     , 17) == 0) CONFIG_LASTMOVE_COLOR_G      = atoi ( cfgline + 17 ); 
			else if (strncmp(cfgline, "lastmove_color_b "     , 17) == 0) CONFIG_LASTMOVE_COLOR_B      = atoi ( cfgline + 17 ); 
			else if (strncmp(cfgline, "error_color_r "        , 14) == 0) CONFIG_ERROR_COLOR_R         = atoi ( cfgline + 14 ); 
			else if (strncmp(cfgline, "error_color_g "        , 14) == 0) CONFIG_ERROR_COLOR_G         = atoi ( cfgline + 14 ); 
			else if (strncmp(cfgline, "error_color_b "        , 14) == 0) CONFIG_ERROR_COLOR_B         = atoi ( cfgline + 14 ); 
			else if (strncmp(cfgline, "movearrow_left_r "     , 17) == 0) CONFIG_MOVEARROW_LEFT_R      = atoi ( cfgline + 17 ); 
			else if (strncmp(cfgline, "movearrow_left_g "     , 17) == 0) CONFIG_MOVEARROW_LEFT_G      = atoi ( cfgline + 17 ); 
			else if (strncmp(cfgline, "movearrow_left_b "     , 17) == 0) CONFIG_MOVEARROW_LEFT_B      = atoi ( cfgline + 17 ); 
			else if (strncmp(cfgline, "movearrow_right_r "    , 18) == 0) CONFIG_MOVEARROW_RIGHT_R     = atoi ( cfgline + 18 ); 
			else if (strncmp(cfgline, "movearrow_right_g "    , 18) == 0) CONFIG_MOVEARROW_RIGHT_G     = atoi ( cfgline + 18 ); 
			else if (strncmp(cfgline, "movearrow_right_b "    , 18) == 0) CONFIG_MOVEARROW_RIGHT_B     = atoi ( cfgline + 18 ); 
			else if (strncmp(cfgline, "lastmove_radius "      , 16) == 0) CONFIG_LASTMOVE_RADIUS       = atoi ( cfgline + 16 ); 
			else if (strncmp(cfgline, "smallcircle_radius "   , 19) == 0) CONFIG_SMALLCIRCLE_RADIUS    = atoi ( cfgline + 19 ); 
			else if (strncmp(cfgline, "circleup_distance "    , 18) == 0) CONFIG_CIRCLEUP_DISTANCE     = atoi ( cfgline + 18 );
			
			else { /* found a config line we don't recognize */
				fprintf (stderr, "Cannot parse config line. ccount = %d.\n"
				                 "\tFull line is: %s\n", ccount, cfgline);
				default_config();
				return 3;
			}
		} while ((ccount != 1) && (ccount != 2));
	
		fclose (fp);
		return 0;
	}
}

