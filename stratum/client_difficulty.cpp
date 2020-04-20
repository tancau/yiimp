
#include "stratum.h"

const double average_period = 4;

double approxRollingAverage(double avg, double new_sample)
{
    avg -= avg / average_period;
    avg += new_sample / average_period;
    return avg;
}

void client_record_difficulty(YAAMP_CLIENT* client)
{
    int timediff = current_timestamp() - client->last_submit_time;
    if (timediff < 250)
        timediff = 250;

    double rough_spm = (client->shares_per_minute + (60000 / timediff)) * 0.485;
    client->shares_per_minute = approxRollingAverage(rough_spm, client->shares_per_minute);
    client->last_submit_time = current_timestamp();
}

void client_adjust_difficulty(YAAMP_CLIENT* client)
{
    //! dont adjust if there hasnt been enough sampletime
    if (time(NULL) - g_last_broadcasted < 5)
        return;

    const double ideal_spm = 100;
    const double spm_maxdrift = 20;
    double current_spm = client->shares_per_minute;
    double current_diff = client->difficulty_actual;
    if (current_spm < ideal_spm - spm_maxdrift || current_spm > ideal_spm + spm_maxdrift) {
        double adjustment_value = current_spm / ideal_spm;
        client_send_difficulty(client, current_diff * adjustment_value);
        client->difficulty_actual = floor((current_diff * adjustment_value) * 65536) / 65536;
    }
}

int client_send_difficulty(YAAMP_CLIENT* client, double difficulty)
{
    if (difficulty < g_stratum_difficulty / 256)
        difficulty = g_stratum_difficulty / 256;
    client_call(client, "mining.set_difficulty", "[%0.8f]", difficulty);
    return 0;
}

void client_initialize_difficulty(YAAMP_CLIENT* client)
{
}
