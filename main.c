#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /* für getopt */
#include <string.h>     /* für strncmp */

#include <ctype.h>      /* wofür? */




#if 1       /* 0 -> test, 1 -> real world */
#define SYSPATH "/sys/devices/platform/sony-laptop"
#else
#define SYSPATH "systest"
#endif

void raise_error(const char *error_message, int exit_now)
{
    fprintf(stderr, "%s", error_message );
    if (exit_now)  exit(EXIT_FAILURE);
}

char * get_thermal_control()
{
    FILE *fp = fopen(SYSPATH "/thermal_control", "r");
    if (fp == NULL) raise_error("file error", 1);
    static char control[12];
    fscanf( fp, "%11s", control);
    fclose(fp);

    return control;
}

void set_thermal_control(const char *control)
{
    FILE *bcl = fopen(SYSPATH "/thermal_control", "w");
    if (bcl == NULL)
        raise_error("file error", 1);


    if (!strcmp(control, "b") || !strcmp(control, "balanced"))
        fprintf( bcl, "balanced" );
    else if (!strcmp(control, "s") || !strcmp(control, "silent"))
        fprintf( bcl, "silent" );
    else if (!strcmp(control, "p") || !strcmp(control, "performance"))
        fprintf( bcl, "performance" );
    else
        raise_error("wrong thermal control. only 'b': 'balanced', 's': 'silent', and 'p': 'performance' are allowed.\n", 0);

    fclose(bcl);
}

int get_battery_care_limiter()
{
    FILE *fp = fopen(SYSPATH "/battery_care_limiter", "r");
    if (fp == NULL) raise_error("file error", 1);

    int i = -1;
    fscanf( fp, "%d", &i);

    fclose(fp);
    return i;
}

void set_battery_care_limiter(const char *level)
{
    FILE *bcl = fopen(SYSPATH "/battery_care_limiter", "w");
    if (bcl == NULL)
        raise_error("file error", 1);


    if (!strcmp(level, "0") || !strcmp(level, "100"))
        fprintf( bcl, "0"  );
    else if (!strcmp(level, "50"))
        fprintf( bcl, "50" );
    else if (!strcmp(level, "80"))
        fprintf( bcl, "80" );
    else
        raise_error("wrong limit. only '0' (== 100), '50', '80' and '100' is allowed\n", 0);

    fclose(bcl);
}

void set_kbd_backlight(char *mode)
{
    FILE *kb  = fopen(SYSPATH "/kbd_backlight", "w");
    FILE *kbt = fopen(SYSPATH "/kbd_backlight_timeout", "w");

    if ( kb == NULL || kbt == NULL)
            raise_error("file error", 1);

    // initialize first
    fprintf(kb,  "1"); fflush(kb);  rewind(kb);
    fprintf(kb,  "0"); fflush(kb);  rewind(kb);
    fprintf(kbt, "1"); fflush(kbt); rewind(kbt);
    fprintf(kbt, "0"); fflush(kbt); rewind(kbt);


    // endless on, if dark
    if (!strcmp( mode, "e") || !strcmp( mode, "endless"))
    {
        fprintf(kb,  "1");
        fprintf(kbt, "0");
    }

    //off
    else if (!strcmp( mode, "n") || !strcmp( mode, "off"))
    {
        fprintf(kb, "0");
    }

    // on after keypress, if dark
    else if (!strcmp( mode, "t") || !strcmp( mode, "t1") || !strcmp( mode, "t2") || !strcmp(mode, "t3") || !strcmp(mode, "timeout"))
    {
        fprintf(kb, "1");
        fprintf(kbt, "3"); rewind(kbt);
        if (!strcmp(mode, "t1"))
            fprintf(kbt, "1");
        if (!strcmp(mode, "t2"))
            fprintf(kbt, "2");
    }

    else
    {
        printf("use -k with \n"
        "   'e' | 'endless' for endless backlight, if dark \n"
        "   'n' | 'off' for no backlight, \n"
        "   't' | 't1' | 't2' | 't3' | 'timeout' for backlight with timout after keypress,\n"
        "                       if dark. Different timeouts can be selected with 't<n>'\n"
        "                              't1': 10s\n"
        "                              't2': 30 s\n"
        "                              't3' | 't' | 'timeout': 60s");
    }

    fclose(kb);
    fclose(kbt);

}


int main (int argc, char **argv)
{
    int c;
    int i = 0;


    opterr = 0;
    while ((c = getopt (argc, argv, "k:Bb:Tt:")) != -1)
    {
        i = 1;
        switch (c)
        {
        case 'k':
            set_kbd_backlight(optarg);
            break;
        case 'B':
            printf("Battery care limiter set to %d%%\n", (get_battery_care_limiter()==0) ? 100 : get_battery_care_limiter());
            break;
        case 'b':
            set_battery_care_limiter(optarg);
            break;
        case 'T':
            printf("Thermal control is set to %s.\n", get_thermal_control());
            break;
        case 't':
            set_thermal_control(optarg);
            break;
        case '?':
            if (optopt == 'b' || optopt == 'k' || optopt == 't')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            return 1;
        default:
            abort ();
        }
    }
    if ( i == 0 )
    {
        printf("Usage: LaptopControl [-b percentage] [-B] -[k mode]\n"
                "-b <percentage>: set battery care limiter to <percentage> percent. Allowed values: 50, 80, 100(=0)\n"
                "-B: Print battery care limiter value. 0 == 100\n"
                "-t <mode>: set thermal control (fan's speed).\n"
                "  mode: 's' | 'silent'\n"
                "        'b' | 'balanced'\n"
                "        'p' | 'performance'\n"
                "-T: Print current thermal control (fan's speed).\n"
                "-k <mode>: set keyboard backlight status and timeout:\n"
                "  mode: 'e' | 'endless' for endless backlight, if dark \n"
                "        'n' | 'off' for no backlight, \n"
                "        't' | 't1' | 't2' | 't3' | 'timeout' for backlight with timout after\n"
                "                       keypress, if dark. Different timeouts can be selected\n"
                "                       with 't<n>'\n"
                "                              't1': 10s\n"
                "                              't2': 30 s\n"
                "                                  't3' | 't' | 'timeout': 60s\n");
      }

  return 0;
}
