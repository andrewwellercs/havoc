#include <stdio.h>

#include <SDL2/SDL_net.h>

#include "../utils/Vector.h"

#include "Server_Pool.h"
#include "Server_Receiver.h"
#include "Server_Sender.h"

#include "../game/Game.h"
#include "../game/Player.h"
#include "../game/Projectile.h"
#include "../utils/Network_utils.h"

#define HAVOC_SERVER

static int delta = 10; //10 ms between update times

int main()
{
    SDL_Init(0);
    SDLNet_Init();
    Pool_init();

    Game_init();

    Server_Receiver_run(23432);
    Server_Sender_run();

    int last_time = SDL_GetTicks();
    while (1) {
        int current_time = SDL_GetTicks();
        //update loop
        while (current_time > last_time + delta) {
            Vector* received = Server_Receiver_get_received();
            while (received->num > 0) {
                UDPpacket* pack = Vector_pop(received);
                int id = SDLNet_Read32(pack->data);
                int type = SDLNet_Read32(pack->data + 4);
                switch (type) {
                case PLAYER_UPDATE:
                    if (!Player_get(id)) {
                        Player_connect("unknown_player", NULL);
                        Player_get(id)->team = id;
                    }
                    Network_decipher_player_packet(pack, Player_get(id), 1);
                    SDLNet_FreePacket(pack);
                    pack = NULL;
                    break;
                case CHANGE_NAME:
                    Network_decipher_change_name_packet(pack);
                    SDLNet_FreePacket(pack);
                    pack = NULL;
                    break;
                case PROJECTILE_LAUNCH:
                    Network_decipher_projectile_packet(pack, NULL);
                    break;
                case GET_NAMES:
                    SDLNet_FreePacket(pack);
                    pack = Network_create_receive_names_packet();
                    break;
                }

                if (pack) {
                    SDL_LockMutex(shared_pool.sending_mutex);
                    Vector_push(shared_pool.sending, pack);
                    SDL_UnlockMutex(shared_pool.sending_mutex);
                }
            }

            Game_update(((float)delta) / 1000.0f);

            //send player information back to the clients
            for (int i = 0; i < Player_num_players(); i++) {
                Player* p = Player_get(i);
                if (p) {
                    UDPpacket* pack = Network_create_player_packet(p);
                    UDPpacket* copy = SDLNet_AllocPacket(pack->maxlen + 4);
                    SDLNet_Write32(i, copy->data);
                    for (int i = 0; i < pack->len; i++) {
                        copy->data[i + 4] = pack->data[i];
                    }
                    copy->len = pack->len + 4;
                    SDLNet_FreePacket(pack);

                    SDL_LockMutex(shared_pool.sending_mutex);
                    Vector_push(shared_pool.sending, copy);
                    SDL_UnlockMutex(shared_pool.sending_mutex);
                }
            }

            //send information about projectiles dying
            for (int i = 0; i < Proj_num_projectiles(); i++) {
                if (Proj_server_should_kill(i)) {
                    Proj_server_do_kill(i);
                    printf("Sending kill packet in server\n");
                    UDPpacket* pack = Network_create_projectile_death_packet(i);

                    SDL_LockMutex(shared_pool.sending_mutex);
                    Vector_push(shared_pool.sending, pack);
                    SDL_UnlockMutex(shared_pool.sending_mutex);
                }
            }

            last_time += delta;
        }
    }

    Server_Receiver_stop();
    Server_Sender_stop();

    Pool_deinit();

    SDLNet_Quit();
    SDL_Quit();
    return 0;
}
