/**
 * MIT License
 *
 * Copyright (c) 2023 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico-w-ble-midi-server-demo-cli.h"
static EmbeddedCli* cli = NULL;

static void onCommand(const char* name, char *tokens)
{
    printf("Received command: %s\r\n",name);

    for (int i = 0; i < embeddedCliGetTokenCount(tokens); ++i) {
        printf("Arg %d : %s\r\n", i, embeddedCliGetToken(tokens, i + 1));
    }
}

static void onSend(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    static uint8_t byte_array[255];
    uint16_t nargs = embeddedCliGetTokenCount(args);
    if (nargs < 1 || nargs > 255) {
        printf("usage: send <space-separated list of up to 255 MIDI Hex digits>\r\n");
        return;
    }
    for (uint16_t idx = 0; idx < nargs; idx++) {
        byte_array[idx] = (uint8_t)strtol(embeddedCliGetToken(args, idx+1), NULL, 16);
    }
    process_send_cb_t cb = (process_send_cb_t)context;
    cb(byte_array, nargs);
}

static void onCommandFn(EmbeddedCli *embeddedCli, CliCommand *command)
{
    (void)embeddedCli;
    embeddedCliTokenizeArgs(command->args);
    onCommand(command->name == NULL ? "" : command->name, command->args);
}

static void writeCharFn(EmbeddedCli *embeddedCli, char c)
{
    (void)embeddedCli;
    putchar(c);
}

EmbeddedCli* pico_w_ble_midi_server_demo_cli_init(process_send_cb_t process_send_cb)
{
    while(getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {
        // flush out the console input buffer
    }
    EmbeddedCliConfig* cfg = embeddedCliDefaultConfig();
    cfg->cmdBufferSize = 128;
    cfg->rxBufferSize = 128;
    // Initialize the CLI
    cli = embeddedCliNew(cfg);
    cli->onCommand = onCommandFn;
    cli->writeChar = writeCharFn;
    CliCommandBinding sendBinding = {
            "send",
            "Send the space-separated HEX digits as MIDI message",
            true,
            process_send_cb,
            onSend
    };
    embeddedCliAddBinding(cli, sendBinding);

    printf("Cli is running.\r\n");
    printf("Type \"help\" for a list of commands\r\n");
    printf("Use backspace and tab to remove chars and autocomplete\r\n");
    printf("Use up and down arrows to recall previous commands\r\n");

    embeddedCliProcess(cli);
    return cli;
}

void pico_w_ble_midi_server_demo_cli_task()
{
    int c = getchar_timeout_us(0);
    if (c != PICO_ERROR_TIMEOUT)
    {
        embeddedCliReceiveChar(cli, c);
        embeddedCliProcess(cli);
    }
}
