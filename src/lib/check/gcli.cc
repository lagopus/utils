#include "lagopus_apis.h"

#include "grpcclistub.h"





static uint32_t s_sleep = 0;
static bool s_do_recv = true;
static const char s_default_uri[] = "localhost:45678";
static char s_uri[4096];


static inline void
parse_args(int argc, const char * const argv[]) {
  while (*argv != NULL) {
    if (strcmp(*argv, "-s") == 0) {
      s_do_recv = false;
    } else if (strcmp(*argv, "-r") == 0) {
      s_do_recv = true;
    } else if (strcmp(*argv, "-u") == 0) {
      if (IS_VALID_STRING(*(argv + 1)) == true) {
        argv++;
        snprintf(s_uri, sizeof(s_uri), "%s", *argv);
      } else {
        lagopus_msg_error("server URI is not specified.\n");        
      }
    } else if (strcmp(*argv, "-w") == 0) {
      if (IS_VALID_STRING(*(argv + 1)) == true) {
        uint32_t tmp;
        argv++;
        if (lagopus_str_parse_uint32(*argv, &tmp) == LAGOPUS_RESULT_OK) {
          s_sleep = tmp;
        } else {
          lagopus_msg_error("invalid wait time '%s'.\n", *argv);
        }
      } else {
        lagopus_msg_error("the wait time is not specified.\n");
      }
    }
    argv++;
  }

  if (IS_VALID_STRING(s_uri) == false) {
    snprintf(s_uri, sizeof(s_uri), "%s",s_default_uri);
  }
}


int
main(int argc, const char * const argv[]) {
  lagopus_result_t st = LAGOPUS_RESULT_ANY_FAILURES;

  parse_args(argc - 1, argv + 1);
  
  st = pio_rpc_client_init(s_uri);
  if (st == LAGOPUS_RESULT_OK) {
    pio_bulk_packets *bp = NULL;

    if (s_do_recv == false) {
      /*
       * Send packets.
       */
      bp = pio_create_bulk_packets(3);
      if (bp != NULL) {
        pio_packet *p;
        size_t i;
        char buf[32];

        for (i = 0; i < 3; i++) {
          snprintf(buf, sizeof(buf), "packet #" PFSZ(u), i);
          p = pio_create_packet((int32_t)i,
                                (uint32_t)(strlen(buf) + 1),
                                (uint8_t *)buf);
          st = pio_set_packet_to_bulk(bp, i, p);
          if (st < 0) {
            goto done;
          }
        }

        st = pio_send_bulk_packets(bp);
        pio_destroy_bulk_packets(bp);
      }
    } else {
      /*
       * Receive packets.
       */
      pio_packet *p;
      size_t i;

      st = pio_recv_bulk_packets(&bp);
      if (st >= LAGOPUS_RESULT_OK) {
        fprintf(stdout, "got " PFSZ(u) " packets:\n", st);
        for (i = 0; i < (size_t)st; i++) {
          p = pio_get_packet_from_bulk(bp, i);
          fprintf(stdout, "\tpacket[" PFSZS(2, u) "]: port %2d: len %4u\n",
                  i,
                  pio_packet_port(p),
                  pio_packet_size(p));
        }
        pio_destroy_bulk_packets(bp);
      }
    }

    if (s_sleep > 0) {
      sleep(s_sleep);
    }
  }

  pio_rpc_client_final();

done:
  if (st < LAGOPUS_RESULT_OK) {
    lagopus_perror(st);
  }
  return (st >= LAGOPUS_RESULT_OK) ? 0 : 1;
}

