#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <alsa/asoundlib.h>

// Fonction pour jouer un son à une fréquence et durée données (en onde carrée)
void play_square_wave(unsigned int sample_rate, float frequency, float duration) {
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    int16_t *buffer;
    int err;

    // Calcul du nombre d'échantillons
    int num_samples = sample_rate * duration;
    
    // Allouer un buffer pour stocker les échantillons audio
    buffer = (int16_t *)malloc(num_samples * sizeof(int16_t));
    if (!buffer) {
        perror("Erreur d'allocation de mémoire pour le buffer");
        return;
    }

    // Initialiser ALSA
    err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "Erreur d'ouverture du périphérique audio : %s\n", snd_strerror(err));
        free(buffer);
        return;
    }

    // Définir les paramètres audio (format, fréquence d'échantillonnage, etc.)
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, 1); // Mono
    snd_pcm_hw_params_set_rate_near(handle, params, &sample_rate, 0);
    snd_pcm_hw_params(handle, params);

    // Générer l'onde carrée
    for (int i = 0; i < num_samples; i++) {
        // Créer une onde carrée : alternance entre +32767 (haut) et -32767 (bas)
        buffer[i] = (i % (int)(sample_rate / frequency) < (int)(sample_rate / frequency) / 2) ? 32767 : -32767;
    }

    // Jouer le son
    snd_pcm_writei(handle, buffer, num_samples);

    // Libérer les ressources
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
}

int main() {
    unsigned int sample_rate = 44100; // Fréquence d'échantillonnage
    float duration = 0.2; // Durée du son en millisecondes (500ms par note)

    // Fréquences des 24 notes de la gamme chromatique (la gamme de C4 à C5)
    float notes[45] = {
        587.3295358348151,
        493.8833012561241,
        659.2551138257398,
        587.3295358348151,
        523.2511306011972,
        587.3295358348151,
        523.2511306011972,
        493.8833012561241,
        659.2551138257398,
        523.2511306011972,
        659.2551138257398,
        440.0,
        659.2551138257398,
        587.3295358348151,
        659.2551138257398,
        493.8833012561241,
        659.2551138257398,
        587.3295358348151,
        523.2511306011972,
        587.3295358348151,
        523.2511306011972,
        493.8833012561241,
        880.0,
        659.2551138257398,
        1318.5102276514797,
        659.2551138257398,
        1318.5102276514797,
        1318.5102276514797,
        659.2551138257398,
        987.7666025122483,
        659.2551138257398,
        1318.5102276514797,
        1046.5022612023945,
        987.7666025122483,
        880.0,
        987.7666025122483,
        880.0,
        1975.533205024496,
        1760.0,
        1567.981743926997,
        1396.9129257320155,
        1318.5102276514797,
        1174.6590716696303,
        1046.5022612023945,
        987.7666025122483,

    };
    int i = 0;
    while(1){
        
        printf("%f \n", notes[i]);
        fflush(stdout);
        play_square_wave(sample_rate, notes[i], duration); // Jouer chaque note pendant 500ms
        i ++;
    }

    return 0;
}
