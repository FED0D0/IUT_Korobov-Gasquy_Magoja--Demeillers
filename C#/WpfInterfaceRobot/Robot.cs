using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WpfApp1
{
    public class Robot
    {
        public string receivedText = "";
        public float distanceTelemetreDroit;
        public float distanceTelemetreCentre;
        public float distanceTelemetreGauche;
        public Queue<byte> byteListReceived;
        internal float positionXOdo;
        internal float positionYOdo;
        public float Con_x;
        public float Con_t;
        public float Mes_x;
        public float Mes_t;
        public float Err_x;
        public float Err_t;
        public float Com_x;
        public float Com_t;
        public float Kp_x;
        public float Kp_t;
        public float Ki_x;
        public float Ki_t;
        public float Kd_x;
        public float Kd_t;
        public float Cor_p_x;
        public float Cor_p_t;
        public float Cor_i_x;
        public float Cor_i_t;
        public float Cor_d_x;
        public float Cor_d_t;
        public float Cor_p_max_x;
        public float Cor_p_max_t;
        public float Cor_i_max_x;
        public float Cor_i_max_t;
        public float Cor_d_max_x;
        public float Cor_d_max_t;

        public Robot()
        {
            byteListReceived = new Queue<byte>();
    }





    }
}
