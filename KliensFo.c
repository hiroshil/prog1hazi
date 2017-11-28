/*
!!!! MINDIG x86-ra buildelj! A MySQl connector nem mûködik 64 biten. !!!!
>> MSBuild Toolset v140-nél kisebb verziója alatt nem feltétlen fordul le. <<
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include "3rdparty\dirent.h"
#include "3rdparty\mysql-connector-c-6.1.11-win32\include\mysql.h"
//#include "3rdparty\curl-7.56.0\builds\libcurl-vc-x86-release-dll-ipv6-sspi-winssl\include\curl\curl.h"
#include <curl\curl.h>

char IsNaN(float f)
{
	return !(f >= -FLT_MAX);
}

//void delay(unsigned int mseconds)
//{
//	clock_t goal = mseconds + clock();
//	while (goal > clock());
//}

static unsigned int to_latin9(const unsigned int code)
{
	/* Code points 0 to U+00FF are the same in both. */
	if (code < 256U)
		return code;
	switch (code) {
	case 0x0152U: return 188U; /* U+0152 = 0xBC: OE ligature */
	case 0x0153U: return 189U; /* U+0153 = 0xBD: oe ligature */
	case 0x0160U: return 166U; /* U+0160 = 0xA6: S with caron */
	case 0x0161U: return 168U; /* U+0161 = 0xA8: s with caron */
	case 0x0178U: return 190U; /* U+0178 = 0xBE: Y with diaresis */
	case 0x017DU: return 180U; /* U+017D = 0xB4: Z with caron */
	case 0x017EU: return 184U; /* U+017E = 0xB8: z with caron */
	case 0x20ACU: return 164U; /* U+20AC = 0xA4: Euro */
	default:      return 256U;
	}
}

/* Convert an UTF-8 string to ISO-8859-15.
* All invalid sequences are ignored.
* Note: output == input is allowed,
* but   input < output < input + length
* is not.
* Output has to have room for (length+1) chars, including the trailing NUL byte.
*/
size_t utf8_to_latin9(char *const output, const char *const input, const size_t length)
{
	unsigned char             *out = (unsigned char *)output;
	const unsigned char       *in = (const unsigned char *)input;
	const unsigned char *const end = (const unsigned char *)input + length;
	unsigned int               c;

	while (in < end)
		if (*in < 128)
			*(out++) = *(in++); /* Valid codepoint */
		else
			if (*in < 192)
				in++;               /* 10000000 .. 10111111 are invalid */
			else
				if (*in < 224) {        /* 110xxxxx 10xxxxxx */
					if (in + 1 >= end)
						break;
					if ((in[1] & 192U) == 128U) {
						c = to_latin9((((unsigned int)(in[0] & 0x1FU)) << 6U)
							| ((unsigned int)(in[1] & 0x3FU)));
						if (c < 256)
							*(out++) = c;
					}
					in += 2;

				}
				else
					if (*in < 240) {        /* 1110xxxx 10xxxxxx 10xxxxxx */
						if (in + 2 >= end)
							break;
						if ((in[1] & 192U) == 128U &&
							(in[2] & 192U) == 128U) {
							c = to_latin9((((unsigned int)(in[0] & 0x0FU)) << 12U)
								| (((unsigned int)(in[1] & 0x3FU)) << 6U)
								| ((unsigned int)(in[2] & 0x3FU)));
							if (c < 256)
								*(out++) = c;
						}
						in += 3;

					}
					else
						if (*in < 248) {        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
							if (in + 3 >= end)
								break;
							if ((in[1] & 192U) == 128U &&
								(in[2] & 192U) == 128U &&
								(in[3] & 192U) == 128U) {
								c = to_latin9((((unsigned int)(in[0] & 0x07U)) << 18U)
									| (((unsigned int)(in[1] & 0x3FU)) << 12U)
									| (((unsigned int)(in[2] & 0x3FU)) << 6U)
									| ((unsigned int)(in[3] & 0x3FU)));
								if (c < 256)
									*(out++) = c;
							}
							in += 4;

						}
						else
							if (*in < 252) {        /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
								if (in + 4 >= end)
									break;
								if ((in[1] & 192U) == 128U &&
									(in[2] & 192U) == 128U &&
									(in[3] & 192U) == 128U &&
									(in[4] & 192U) == 128U) {
									c = to_latin9((((unsigned int)(in[0] & 0x03U)) << 24U)
										| (((unsigned int)(in[1] & 0x3FU)) << 18U)
										| (((unsigned int)(in[2] & 0x3FU)) << 12U)
										| (((unsigned int)(in[3] & 0x3FU)) << 6U)
										| ((unsigned int)(in[4] & 0x3FU)));
									if (c < 256)
										*(out++) = c;
								}
								in += 5;

							}
							else
								if (*in < 254) {        /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
									if (in + 5 >= end)
										break;
									if ((in[1] & 192U) == 128U &&
										(in[2] & 192U) == 128U &&
										(in[3] & 192U) == 128U &&
										(in[4] & 192U) == 128U &&
										(in[5] & 192U) == 128U) {
										c = to_latin9((((unsigned int)(in[0] & 0x01U)) << 30U)
											| (((unsigned int)(in[1] & 0x3FU)) << 24U)
											| (((unsigned int)(in[2] & 0x3FU)) << 18U)
											| (((unsigned int)(in[3] & 0x3FU)) << 12U)
											| (((unsigned int)(in[4] & 0x3FU)) << 6U)
											| ((unsigned int)(in[5] & 0x3FU)));
										if (c < 256)
											*(out++) = c;
									}
									in += 6;

								}
								else
									in++;               /* 11111110 and 11111111 are invalid */

														/* Terminate the output string. */
	*out = '\0';

	return (size_t)(out - (unsigned char *)output);
}

typedef struct
{
	int Tipus, Emelet, Statusz, Hossz;
} logsor;

typedef struct
{
	int Tipus, Emelet, OsszMukodes, OsszPihenes, OsszNincsadat, OsszLogHossz;
	float MukodesArany;//Logolt idõ mekkora részében ment
} Eszkoz;

MYSQL *conn;
int SqlConnectAndInit()
{
	if ((conn = mysql_init(NULL)) == NULL)
	{
		fprintf(stderr, "Could not init DB\n");
		return 1;
	}
	if (mysql_real_connect(conn, "sql11.freemysqlhosting.net", "sql11197729", "qVXXNgHgu8", "sql11197729", 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "DB Connection Error\n");
		return 2;
	}
	return 0;
}
int SqlCloseHandle()
{
	mysql_close(conn);
}

char querySqlInsert[800];
int SqlInsert(logsor sor)
{
	//printf("\nSqlInsert() 0\n");
	sprintf(querySqlInsert, "INSERT INTO geplog (Tipus, Emelet, Statusz, Hossz) VALUES ('%d', '%d', '%d', '%d')", sor.Tipus, sor.Emelet, sor.Statusz, sor.Hossz);
	//printf("SqlInsert() 1\n");
	if (mysql_query(conn, querySqlInsert) != 0)
	{
		fprintf(stderr, "Query Failure\n");
		return EXIT_FAILURE;
	}
	//printf("SqlInsert() 2\n");
	return 0;
}


MYSQL_RES *res;
MYSQL_ROW row;

// Declaration
Eszkoz* Eszkozok = 0;
Eszkoz* EszkozokBuffer = 0;
int EszkozokSzama = 0;

void EszkozTombNoveles(int proba)
{
	if (proba > 10)
	{
		printf("Multiple failures during memory allocation!\n");
		return;
	}

	/*if (Eszkozok != 0)
	{
		Eszkozok = (int*)realloc(Eszkozok, EszkozokSzama * sizeof(int));
	}
	else
	{
		Eszkozok = (int*)malloc(EszkozokSzama * sizeof(int));
	}*/

	++EszkozokSzama;

	//if (EszkozokSzama == 1) return;
	// Make the array bigger
	EszkozokBuffer = (Eszkoz*)realloc(Eszkozok, EszkozokSzama * sizeof(Eszkoz));
	if (!EszkozokBuffer) { --EszkozokSzama; Sleep(10); EszkozTombNoveles(proba + 1); return; }

	Eszkozok = EszkozokBuffer;



	//init new element
	Eszkozok[EszkozokSzama - 1].OsszMukodes = 0;
	Eszkozok[EszkozokSzama - 1].OsszPihenes = 0;
	Eszkozok[EszkozokSzama - 1].OsszNincsadat = 0;
	Eszkozok[EszkozokSzama - 1].OsszLogHossz = 0;
	Eszkozok[EszkozokSzama - 1].Emelet = 0;
	Eszkozok[EszkozokSzama - 1].Tipus = 0;
	Eszkozok[EszkozokSzama - 1].MukodesArany = 0;

	//printf_s("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
	// Clean up when you're done.
	//free(EszkozokBuffer);
}

int GetEszkozIndex(int Emelet, int Tipus)//Ha nincs még ilyen a tömbben, akkor -1
{
	int i = 0;
	for (; i < EszkozokSzama; i++)
	{
		if (Eszkozok[i].Emelet == Emelet && Eszkozok[i].Tipus == Tipus)
			return i;
	}
	return -1;
}

int CompareEszkozEmelet(const void * a, const void * b)
{

	Eszkoz *orderA = (Eszkoz *)a;
	Eszkoz *orderB = (Eszkoz *)b;

	return (orderA->Emelet - orderB->Emelet);
}

void EszkozKiiro(Eszkoz eszk)
{
	char *veg = "th";
	if (eszk.Emelet % 100 < 10 || eszk.Emelet % 100 > 20)
	{
		if (eszk.Emelet % 10 == 1)
			veg = "st";
		else if (eszk.Emelet % 10 == 2)
			veg = "nd";
		else if (eszk.Emelet % 10 == 3)
			veg = "rd";
	}

	printf("=>%d%s floor:\n", eszk.Emelet, veg);
	if (IsNaN(eszk.MukodesArany))
		printf("   -=>Utilization: N/A\n");
	else
		printf("   -=>Utilization: %f%%\n", eszk.MukodesArany * 100);

	printf("   -=>Logged time: %ds\n      -->Active duration: %ds\n      -->Inactive duration: %ds\n      -->N/A duration: %ds\n\n", eszk.OsszLogHossz, eszk.OsszMukodes, eszk.OsszPihenes, eszk.OsszNincsadat);
}

void StatisztikaKeszitesKiiras()
{
	float atlagMosogep = 0, atlagSzarito = 0, abszelterMosogep = 0, abszelterSzarito = 0;
	int MosogepSzam = 0, SzaritoSzam = 0;

	int i;

	qsort(Eszkozok, EszkozokSzama, sizeof(Eszkoz), CompareEszkozEmelet);

	for (i = 0; i < EszkozokSzama; ++i)
	{
		Eszkozok[i].MukodesArany = (float)Eszkozok[i].OsszMukodes / (float)(Eszkozok[i].OsszMukodes + Eszkozok[i].OsszPihenes);
		Eszkozok[i].OsszLogHossz = Eszkozok[i].OsszMukodes + Eszkozok[i].OsszPihenes + Eszkozok[i].OsszNincsadat;

		if (!IsNaN(Eszkozok[i].MukodesArany))
		{
			if (Eszkozok[i].Tipus == 1)
			{
				++MosogepSzam;
				atlagMosogep += Eszkozok[i].MukodesArany;
			}
			else if (Eszkozok[i].Tipus == 0)
			{
				++SzaritoSzam;
				atlagSzarito += Eszkozok[i].MukodesArany;

			}
		}
	}

	if (MosogepSzam > 0)
		atlagMosogep /= (float)MosogepSzam;

	if (SzaritoSzam > 0)
		atlagSzarito /= (float)SzaritoSzam;

	for (i = 0; i < EszkozokSzama; ++i)
	{
		if (!IsNaN(Eszkozok[i].MukodesArany))
		{
			if (Eszkozok[i].Tipus == 1 && MosogepSzam > 0)
			{
				abszelterMosogep += fabs(atlagMosogep - Eszkozok[i].MukodesArany);
			}
			else if (Eszkozok[i].Tipus == 0 && SzaritoSzam > 0)
			{
				abszelterSzarito += fabs(atlagSzarito - Eszkozok[i].MukodesArany);

			}
		}
	}

	if (MosogepSzam > 0)
	{

		abszelterMosogep /= (float)MosogepSzam;

		printf("----------\n");
		printf("=Washing Machines:\n");
		printf("==Average Utilisation: %f%%\n", atlagMosogep * 100);
		printf("==Average Absolute Deviation: %f%%\n", abszelterMosogep * 100);
		printf("==Floors With Outstanding Values:\n");
		for (i = 0; i < EszkozokSzama; ++i)
		{
			if (!IsNaN(Eszkozok[i].MukodesArany))
			{
				if (Eszkozok[i].Tipus == 1)
				{
					if (Eszkozok[i].MukodesArany > atlagMosogep + abszelterMosogep)
					{
						printf("===>>%d\n", Eszkozok[i].Emelet);
					}
				}
			}
		}
		printf("----------\n");
	}

	if (SzaritoSzam > 0)
	{

		abszelterSzarito /= (float)SzaritoSzam;

		printf("----------\n");
		printf("=Dryers:\n");
		printf("==Average Utilisation: %f%%\n", atlagSzarito * 100);
		printf("==Average Absolute Deviation: %f%%\n", abszelterSzarito * 100);
		printf("==Floors With Outstanding Values:\n");
		for (i = 0; i < EszkozokSzama; ++i)
		{
			if (!IsNaN(Eszkozok[i].MukodesArany))
			{
				if (Eszkozok[i].Tipus == 0)
				{
					if (Eszkozok[i].MukodesArany > atlagSzarito + abszelterSzarito)
					{
						printf("===>>%d\n", Eszkozok[i].Emelet);
					}
				}
			}
		}
		printf("----------\n");
	}


	printf("\n-------------------------------------------------------\n");
	printf("=Washing Machines:\n");
	printf("-------------------------------------------------------\n");
	{
		for (i = 0; i < EszkozokSzama; i++)
		{
			if (Eszkozok[i].Tipus == 1)
				EszkozKiiro(Eszkozok[i]);

		}


		printf("\n-------------------------------------------------------\n");
		printf("=Dryers:\n");
		printf("-------------------------------------------------------\n");
		for (i = 0; i < EszkozokSzama; i++)
		{
			if (Eszkozok[i].Tipus == 0)
				EszkozKiiro(Eszkozok[i]);

		}
	}

	printf("\n              >>>>>>>>>>> SCROLL TO THE TOP TO SEE THE REPORT! <<<<<<<<<<<\n\n\n");
}

void Felugyelet()
{
	printf("MODE: Online Supervision\n");

	// Allocation (let's suppose size contains some value discovered at runtime,
	// e.g. obtained from some external source)

	Eszkozok = (Eszkoz*)malloc(0 * sizeof(Eszkoz));


	if (mysql_query(conn, "SELECT * FROM geplog") == 0)
	{
		int x;

		res = mysql_store_result(conn);

		//int num_fields = mysql_num_fields(res);
		printf("\nSQl query resulted in %d rows.\n\n", mysql_num_rows(res));

		while ((row = mysql_fetch_row(res)))
		{
			x = GetEszkozIndex(atoi(row[2]), atoi(row[1]));

			if (x == -1)
			{
				EszkozTombNoveles(1);
				x = EszkozokSzama - 1;

				Eszkozok[x].Emelet = atoi(row[2]);
				Eszkozok[x].Tipus = atoi(row[1]);
			}

			if (atoi(row[3]) == 0)//Inaktív
				Eszkozok[x].OsszPihenes += atoi(row[4]);
			else if (atoi(row[3]) == 1)//Aktív
				Eszkozok[x].OsszMukodes += atoi(row[4]);
			else //N/
				Eszkozok[x].OsszNincsadat += atoi(row[4]);

			//int n;
			//// Print all columns
			//for (n = 0; n < num_fields; n++)
			//{
			//	// Make sure row[i] is valid!
			//	if (row[n] != NULL)
			//		printf("%s   ", row[n]);
			//	else
			//		printf("NULL");
			//}
			//printf("\n");
		}

		// DON'T FORGET TO CLEAN RESULT AFTER YOU DON'T NEED IT 
		// ANYMORE
		if (res != NULL)
			mysql_free_result(res);

		StatisztikaKeszitesKiiras();
		//printf("\n");

		//SELECT * FROM licenszek WHERE LicID = ? 
	}
	else
		printf("Error during Sql query\n");

	//printf("Press enter to exit!\n");
	//char enter = 0;
	//scanf_s("%c", &enter);
	//while (enter != '\r' && enter != '\n') { enter = getchar(); }
}

struct string {
	char *ptr;
	size_t len;
};

void init_string(struct string *s) {
	s->len = 0;
	s->ptr = (char*)malloc(s->len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = (char*)realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

int BreakI;

void GetSubString(char *s, int startInclusive, int hossz, char *ki)
{
	//printf("{{%s}}", ki);
	BreakI = startInclusive;
	for (; BreakI < startInclusive + hossz; ++BreakI)
	{
		ki[BreakI - startInclusive] = s[BreakI];
	}
	//printf("{{%s}}", ki);
}
char StartsWith(struct string s, int startindex, char *teszt, int hossz)
{
	BreakI = startindex;
	for (; BreakI < startindex + hossz; ++BreakI)
	{
		if (s.ptr[BreakI] != teszt[BreakI - startindex])
			return 0;
	}
	return 1;
}

unsigned int logdelay = 5 * 60;//s
unsigned int offlinelogdelay = 5 * 60;//s
char *szambuff1;//Mert az eltérõ emeletszám hosszok miatti sokszori memória allokálással bizonytalanul futott
char *szambuff2;
char *szambuff3;
char *szambuff4;
char *szambuff5;
char *szambuff6;

void EnumString(struct string s, char OnlineMode /*0: Offline, Más: Online*/)
{
	int Emelet = -99999;
	int szamhossz;
	char logolj;
	logsor l;
	int i = 0;
	int x;
	printf("  EnumString() --> s.len = %d\n", s.len);
	for (; i < s.len; ++i)
	{
		if (StartsWith(s, i, " id=\"f", 6))//Új emelet indexe
		{
			i += 6;
			if (!isdigit(s.ptr[i]) && !s.ptr[i] == '-')
				continue;
			szamhossz = 0;
			for (; i < s.len; ++i)
			{
				if (isdigit(s.ptr[i]) || s.ptr[i] == '-')
					++szamhossz;
				else
					break;
			}

			if (i >= s.len)
				return;

			//szambuff = (char*)malloc(szamhossz * sizeof(char));
			switch (szamhossz)
			{
			case 1:
			{
				GetSubString(s.ptr, i - szamhossz, szamhossz, szambuff1);
				Emelet = atoi(szambuff1);
				break;
			}
			case 2:
			{
				GetSubString(s.ptr, i - szamhossz, szamhossz, szambuff2);
				Emelet = atoi(szambuff2);
				break;
			}
			case 3:
			{
				GetSubString(s.ptr, i - szamhossz, szamhossz, szambuff3);
				Emelet = atoi(szambuff3);
				break;
			}
			case 4:
			{
				GetSubString(s.ptr, i - szamhossz, szamhossz, szambuff4);
				Emelet = atoi(szambuff4);
				break;
			}
			case 5:
			{
				GetSubString(s.ptr, i - szamhossz, szamhossz, szambuff5);
				Emelet = atoi(szambuff5);
				break;
			}
			case 6:
			{
				GetSubString(s.ptr, i - szamhossz, szamhossz, szambuff6);
				Emelet = atoi(szambuff6);
				break;
			}
			}

			if (Emelet > 18)
			{
				printf("                      >>>>>>>>>>>> DEBUG: Emelet: %d i: %d <<<<<<<<<<<<", Emelet, i);
			}

			//printf("{-{%s}-}", szambuff);
			//printf("{-{%d}-}", atoi(&szambuff));
			//printf("{{{%d}}}", atoi(&szambuff));
		}

		logolj = 0;
		//if (StartsWith(s, i, "Mos", 3))
		if (StartsWith(s, i, "Mosógép: Foglalt!", 17))//Új eszköz indexe
		{
			i += 17;
			l.Tipus = 1;
			l.Statusz = 1;
			logolj = 1;
		}
		else if (StartsWith(s, i, "Mosógép: Szabad!", 16))//Új eszköz indexe
		{
			i += 16;
			l.Tipus = 1;
			l.Statusz = 0;
			logolj = 1;
		}
		else if (StartsWith(s, i, "Mosógép: Nincs információ!", 26))//Új eszköz indexe
		{
			i += 26;
			l.Tipus = 1;
			l.Statusz = 2;
			logolj = 1;
		}
		else if (StartsWith(s, i, "Szárító: Foglalt!", 17))//Új eszköz indexe
		{
			i += 17;
			l.Tipus = 0;
			l.Statusz = 1;
			logolj = 1;
		}
		else if (StartsWith(s, i, "Szárító: Szabad!", 16))//Új eszköz indexe
		{
			i += 16;
			l.Tipus = 0;
			l.Statusz = 0;
			logolj = 1;
		}
		else if (StartsWith(s, i, "Szárító: Nincs információ!", 26))//Új eszköz indexe
		{
			i += 26;
			l.Tipus = 0;
			l.Statusz = 2;
			logolj = 1;
		}

		if (logolj)
		{
			if (OnlineMode)
			{
				l.Emelet = Emelet;
				l.Hossz = logdelay;

				Sleep(30);

				printf("  ==>Creating row: Type: %d  Floor: %d  Status: %d  Duration: %ds   -> ", l.Tipus, l.Emelet, l.Statusz, l.Hossz);
				if (SqlInsert(l) == 0)
					printf("Succeeded.\n");
				else
					printf("Failed.\n");
			}
			else
			{
				//printf_s("FSZMTEST\n");
				x = GetEszkozIndex(Emelet, l.Tipus);
				if (x == -1)
				{
					EszkozTombNoveles(1);
					x = EszkozokSzama - 1;

					Eszkozok[x].Emelet = Emelet;
					Eszkozok[x].Tipus = l.Tipus;
				}

				if (l.Statusz == 0)//Inaktív
					Eszkozok[x].OsszPihenes += offlinelogdelay;
				else if (l.Statusz == 1)//Aktív
					Eszkozok[x].OsszMukodes += offlinelogdelay;
				else //N/A
					Eszkozok[x].OsszNincsadat += offlinelogdelay;


				printf_s("  ==>Gathered data: Type: %d  Floor: %d  Status: %d\n", l.Tipus, Emelet, l.Statusz);
			}
		}

	}
	//free(szambuff);
}

CURL *curl;
struct string sEgyLog;
struct string xEgyLog;
void EgyLogKeszito()
{
	int dekodolthossz;
	printf("Creating new log...\n============================\n");


	printf("cURL return code: %d\n", curl_easy_perform(curl));
	//printf("EgyLogKeszito() 1\n");

	/*if (sEgyLog.len == 0)
	{
		printf("cURL Error: result length is 0.\n");
		curl_easy_cleanup(curl);
		curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_URL, "http://mosogep.sch.bme.hu/index.php");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sEgyLog);
		curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");

	}*/

	//x.len = 10000;
	xEgyLog.ptr = (char*)realloc(xEgyLog.ptr, sEgyLog.len * sizeof(char));//Ez biztosan nagyobb, vagy akkora, mint a dekódolás utáni karakterlánc

	xEgyLog.len = utf8_to_latin9(xEgyLog.ptr, sEgyLog.ptr, sEgyLog.len);


	//writefunc("ABCDEFABCDEF", dekodolthossz, 1, &xEgyLog);

	//printf("INSTRING: %s", sEgyLog.ptr);

	//printf("EgyLogKeszito() 2\n");
	EnumString(xEgyLog, 1);

	//printf("%s\n", xEgyLog.ptr);
	//free(sEgyLog.ptr);

	/* always cleanup */
	//curl_easy_cleanup(curl);

	printf("============================\nNew log created.\n");
}

void Logolas()
{
	int i = 0;

	printf("MODE: Online Logging\n");


	curl_global_init(CURL_GLOBAL_ALL);

	for (; i < 5; i++)
	{
		curl = curl_easy_init();
		if (curl)
			break;
	}
	if (i >= 5)
	{
		printf("Curl Initialisation failed 5 times.\nCannot start logging.\nRestart the software and try again!");
		return;
	}


	init_string(&sEgyLog);
	init_string(&xEgyLog);

	szambuff1 = (char*)malloc(1 * sizeof(char));
	szambuff2 = (char*)malloc(2 * sizeof(char));
	szambuff3 = (char*)malloc(3 * sizeof(char));
	szambuff4 = (char*)malloc(4 * sizeof(char));
	szambuff5 = (char*)malloc(5 * sizeof(char));
	szambuff6 = (char*)malloc(6 * sizeof(char));

	curl_easy_setopt(curl, CURLOPT_URL, "http://mosogep.sch.bme.hu/index.php");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");

	while (1)
	{
		//init_string(&sEgyLog);
		//init_string(&xEgyLog);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sEgyLog);
		EgyLogKeszito();
		printf("Delaying %ds ... ", logdelay);
		//sEgyLog.len = 0;//Különben a következõ WebRequest ehhez (az elõzõhöz) írná hozzá az eredményt
		free(sEgyLog.ptr);
		//xEgyLog.len = 0;
		//free(xEgyLog.ptr);
		Sleep((unsigned int)(logdelay * 1000));
		printf("Done.\n");
		init_string(&sEgyLog);
	}


	curl_easy_cleanup(curl);//Jelenleg a while(1) miatt felesleges
	free(szambuff1);
	free(szambuff2);
	free(szambuff3);
	free(szambuff4);
	free(szambuff5);
	free(szambuff6);
	//logsor l = { 0, 2, 1, 3 };
	//SqlInsert(l);
}

int strend(const char *s, const char *t)
{
	size_t ls = strlen(s); // find length of s
	size_t lt = strlen(t); // find length of t
	if (ls >= lt)  // check if t can fit in s
	{
		// point s to where t should start and compare the strings from there
		return (0 == memcmp(t, s + (ls - lt), lt));
	}
	return 0; // t was longer than s
}

int GetHossz(char *p, int h)
{
	int i = 0;
	for (; i < h; ++i)
	{
		if (p[i] == 0)
			return i;
	}

	return -1;
}

void OfflineMod()
{
	FILE *f;
	DIR *dir;
	struct dirent *ent;
	long fsize, fsizeelozo = 10;
	char *fajltart = (char*)malloc(10);
	char *olvasandofile = (char*)malloc(10);

	printf("MODE: Offline Analytics\n");
	printf("\nCreating logs from '.\\offline\\*.html' files:\n\n");


	szambuff1 = (char*)malloc(1 * sizeof(char));
	szambuff2 = (char*)malloc(2 * sizeof(char));
	szambuff3 = (char*)malloc(3 * sizeof(char));
	szambuff4 = (char*)malloc(4 * sizeof(char));
	szambuff5 = (char*)malloc(5 * sizeof(char));
	szambuff6 = (char*)malloc(6 * sizeof(char));

	Eszkozok = (Eszkoz*)malloc(0 * sizeof(Eszkoz));
	//init_string(&sEgyLog);
	init_string(&xEgyLog);

	if ((dir = opendir("offline")) != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			if (strend(ent->d_name, ".html"))
			{
				printf("FILE: '%s'\n", ent->d_name);

				olvasandofile = (char*)realloc(olvasandofile, 9 + ent->d_namlen);
				strcpy_s(olvasandofile, 9, "offline\\");
				strcat_s(olvasandofile, 9 + ent->d_namlen, ent->d_name);

				fopen_s(&f, olvasandofile, "rb");
				fseek(f, 0, SEEK_END);
				fsize = ftell(f) + 1;
				fseek(f, 0, SEEK_SET);  //same as rewind(f);

				if (fsize > fsizeelozo)
				{
					fajltart = (char*)realloc(fajltart, fsize + 1);
					xEgyLog.ptr = (char*)realloc(xEgyLog.ptr, fsize * sizeof(char));//Ez biztosan nagyobb, vagy akkora, mint a dekódolás utáni karakterlánc
				}
				fsizeelozo = fsize;

				Sleep(10);

				fread(fajltart, fsize, 1, f);
				fclose(f);

				//sEgyLog.len = GetHossz(sEgyLog.ptr, sEgyLog.len);

				fajltart[fsize - 1] = 0;

				xEgyLog.len = utf8_to_latin9(xEgyLog.ptr, fajltart, fsize);
				EnumString(xEgyLog, 0);
				printf("\n");

				fsize = fsize;//DEBUG

				//sEgyLog.len = 0;//Különben a következõ WebRequest ehhez (az elõzõhöz) írná hozzá az eredményt
				//free(sEgyLog.ptr);
				//init_string(&sEgyLog);
			}

			//ent->d_name;
		}
		closedir(dir);
		free(olvasandofile);
		free(fajltart);

		free(szambuff1);
		free(szambuff2);
		free(szambuff3);
		free(szambuff4);
		free(szambuff5);
		free(szambuff6);

		//printf("XXXXXXX\n");
		StatisztikaKeszitesKiiras();
		//printf("YYYYY\n");

	}
	else
	{
		/* could not open directory */
		//perror("");

		printf("\nFile handling error... Directory cannot be opened!\n");

		//return EXIT_FAILURE;
	}

}
void EgyetBeallitInt(char *nev, unsigned int *valtozo)
{
	int be = 0;

	printf("Set %s: ", nev);
	scanf_s("%d", &be);

	*valtozo = be;
	system("cls");
	printf(">>%s has been set to %d sec.\n\n", nev, *valtozo);
}
void Beallitasok()
{
	int be = 0;


	printf("SETTINGS:\n\n");
	printf("1: Logging interval (%d sec)\n", logdelay);
	printf("2: Statistical interval of offline files (%d sec)\n", offlinelogdelay);
	printf("Other: Back\n");

	scanf_s("%d", &be);


	printf("\n\n");
	switch (be)
	{
		case 1:
		{
			/*printf("Set logging interval (sec): ");
			scanf_s("%d", &be);

			logdelay = be;
			system("cls");
			printf(">>Logging interval has been set to %d sec.\n\n", logdelay);*/

			EgyetBeallitInt("Logging interval", &logdelay);
			Beallitasok();
			break;
		}
		case 2:
		{
			/*printf("Set offline logging interval (sec): ");
			scanf_s("%d", &be);

			offlinelogdelay = be;
			system("cls");
			printf(">>Offline logging interval has been set to %d sec.\n\n", offlinelogdelay);*/

			EgyetBeallitInt("Offline logging interval", &offlinelogdelay);
			Beallitasok();
			break;
		}
	}
}
int DisplayMainMenu()
{
	int mod = 0;

	printf("Select mode:\n\n");
	printf("0: Settings.\n\n");
	printf("1: Online Logging - Get HTTP Request from mosogep.sch.bme.hu/index.php and log into online database until the terminal is closed.\n\n");
	printf("2: Online Supervision - Calculate supervision data from online database.\n\n");
	printf("3: Offline Analytics - read '.\\offline\\*.html' files to create a local log. Every file is logged as %d sec duration. After creating the log, the software shows the calculated supervision data immediately.\n\n", offlinelogdelay);
	printf("Other: Exit\n");

	scanf_s("%d", &mod);

	return mod;
}
int LoadSettings()
{
	int hiba = 0;

	FILE *f;
	if (fopen_s(&f, "prefs.sv", "r") != 0)
		return 1;

	if (fread(&logdelay, sizeof(unsigned int), 1, f) != 1)
		hiba = 1;
	if (fread(&offlinelogdelay, sizeof(unsigned int), 1, f) != 1)
		hiba = 1;

	fclose(f);

	return hiba;
}
int SaveSettings()
{
	FILE *f;
	if (fopen_s(&f, "prefs.sv", "w") != 0)
		return 1;

	if (fwrite(&logdelay, sizeof(unsigned int), 1, f) != 1)
		return 1;
	if (fwrite(&offlinelogdelay, sizeof(unsigned int), 1, f) != 1)
		return 1;

	return fclose(f);
}
int main()
{
	int mod;

	if (LoadSettings() != 0)
	{
		printf("Warning: Settings couldn't be loaded properly.\n\n");
	}

	mod = DisplayMainMenu();

	if (mod == 0)
	{
		system("cls");
		Beallitasok();
		system("cls");
		if (SaveSettings() != 0)
		{
			printf("Error: Settings couldn't be saved.\n\n");
		}
		main();
		return;
	}

	if (mod == 1 || mod == 2)
	{
		printf("Initialising MySql connection...\n");
		if (SqlConnectAndInit() == 0)
		{
			printf("Done.\n\n======================\n");

			system("cls");

			if (mod == 1)
			{
				Logolas();
			}
			else if (mod == 2)
			{
				Felugyelet();
			}
		}
		else
		{
			printf("Initialisation failed!\n");
			//scanf("%c");
			return 0;
		}


		//printf("\n\n======================\nClosing Sql Connection Handler...\n");
		//SqlCloseHandle();//NEM szükséges, mert úgyis kilép a program
		//printf("Done.\n");

	}
	else if (mod == 3)
	{
		system("cls");
		OfflineMod();
	}
	free(Eszkozok);
	//free(EszkozokBuffer);//Nem szökséges, mert az 'Eszkozok'-kel együtt felszabadul

	return 0;
}
