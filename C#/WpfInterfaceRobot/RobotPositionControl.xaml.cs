using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace WpfInterfaceRobot
{
    public partial class RobotPositionControl : UserControl
    {
        // ============================================================
        // CONFIGURATION
        // ============================================================

        // Nombre de pixels pour 1 mètre.
        public double PixelsPerMeter { get; set; } = 50.0;


        // ============================================================
        // DONNEES ROBOT
        // ============================================================

        private double robotX = 0.0;
        private double robotY = 0.0;
        private double robotTheta = 0.0;


        // ============================================================
        // DONNEES WAYPOINT
        // ============================================================

        private double waypointX = 0.0;
        private double waypointY = 0.0;
        private double waypointTheta = 0.0;


        // ============================================================
        // CONSTRUCTEUR
        // ============================================================

        public RobotPositionControl()
        {
            InitializeComponent();

            Loaded += RobotPositionControl_Loaded;
        }


        private void RobotPositionControl_Loaded(
            object sender,
            RoutedEventArgs e)
        {
            Redraw();
        }


        // ============================================================
        // MISE A JOUR DU ROBOT
        // ============================================================

        /// <summary>
        /// Affiche le robot à la position demandée.
        ///
        /// x     : mètres
        /// y     : mètres
        /// theta : degrés
        /// </summary>
        public void SetRobotPosition(
            double x,
            double y,
            double theta)
        {
            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.Invoke(() =>
                    SetRobotPosition(x, y, theta));

                return;
            }

            robotX = x;
            robotY = y;
            robotTheta = theta;

            UpdateRobot();
        }


        // ============================================================
        // MISE A JOUR DU WAYPOINT
        // ============================================================

        /// <summary>
        /// Affiche le waypoint.
        ///
        /// x     : mètres
        /// y     : mètres
        /// theta : degrés
        /// </summary>
        public void SetWaypoint(
            double x,
            double y,
            double theta)
        {
            if (!Dispatcher.CheckAccess())
            {
                Dispatcher.Invoke(() =>
                    SetWaypoint(x, y, theta));

                return;
            }

            waypointX = x;
            waypointY = y;
            waypointTheta = theta;

            UpdateWaypoint();
        }


        // ============================================================
        // CONVERSION MONDE -> ECRAN
        // ============================================================

        private Point WorldToScreen(
            double x,
            double y)
        {
            double centerX =
                GraphCanvas.ActualWidth / 2.0;

            double centerY =
                GraphCanvas.ActualHeight / 2.0;

            double screenX =
                centerX + x * PixelsPerMeter;

            // Le Y de WPF est inversé par rapport au repère robot.
            double screenY =
                centerY - y * PixelsPerMeter;

            return new Point(
                screenX,
                screenY);
        }


        // ============================================================
        // ROBOT
        // ============================================================

        private void UpdateRobot()
        {
            if (GraphCanvas.ActualWidth <= 0 ||
                GraphCanvas.ActualHeight <= 0)
            {
                return;
            }

            Point position =
                WorldToScreen(
                    robotX,
                    robotY);


            // --------------------------------------------------------
            // POSITION
            // --------------------------------------------------------

            // Le point (0,0) du Polygon est le barycentre
            // du robot.

            Canvas.SetLeft(
                RobotArrow,
                position.X);

            Canvas.SetTop(
                RobotArrow,
                position.Y);


            // --------------------------------------------------------
            // ORIENTATION
            // --------------------------------------------------------

            // Convention :
            //
            // 0°   -> +X
            // 90°  -> +Y
            // 180° -> -X
            //
            // WPF possède un axe Y inversé,
            // d'où le -theta.

            RobotArrow.RenderTransform =
                new RotateTransform(
                    -robotTheta,
                    0,
                    0);


            // --------------------------------------------------------
            // CENTRE
            // --------------------------------------------------------

            Canvas.SetLeft(
                RobotCenter,
                position.X - RobotCenter.Width / 2);

            Canvas.SetTop(
                RobotCenter,
                position.Y - RobotCenter.Height / 2);
        }


        // ============================================================
        // WAYPOINT
        // ============================================================

        private void UpdateWaypoint()
        {
            if (GraphCanvas.ActualWidth <= 0 ||
                GraphCanvas.ActualHeight <= 0)
            {
                return;
            }

            Point position =
                WorldToScreen(
                    waypointX,
                    waypointY);


            // --------------------------------------------------------
            // POINT
            // --------------------------------------------------------

            Canvas.SetLeft(
                WaypointMarker,
                position.X - WaypointMarker.Width / 2);

            Canvas.SetTop(
                WaypointMarker,
                position.Y - WaypointMarker.Height / 2);


            // --------------------------------------------------------
            // DIRECTION
            // --------------------------------------------------------

            double length = 20.0;

            double angleRad =
                waypointTheta * Math.PI / 180.0;

            double endX =
                position.X +
                Math.Cos(angleRad) * length;

            double endY =
                position.Y -
                Math.Sin(angleRad) * length;

            WaypointDirection.X1 =
                position.X;

            WaypointDirection.Y1 =
                position.Y;

            WaypointDirection.X2 =
                endX;

            WaypointDirection.Y2 =
                endY;
        }


        // ============================================================
        // GRILLE
        // ============================================================

        private void DrawGrid()
        {
            GridCanvas.Children.Clear();

            double width =
                GraphCanvas.ActualWidth;

            double height =
                GraphCanvas.ActualHeight;

            if (width <= 0 ||
                height <= 0)
            {
                return;
            }

            double centerX =
                width / 2.0;

            double centerY =
                height / 2.0;


            // --------------------------------------------------------
            // LIGNES VERTICALES
            // --------------------------------------------------------

            int minX =
                (int)Math.Floor(
                    -centerX / PixelsPerMeter);

            int maxX =
                (int)Math.Ceiling(
                    centerX / PixelsPerMeter);

            for (int x = minX;
                 x <= maxX;
                 x++)
            {
                double screenX =
                    centerX +
                    x * PixelsPerMeter;

                Line line =
                    new Line
                    {
                        X1 = screenX,
                        Y1 = 0,
                        X2 = screenX,
                        Y2 = height,

                        Stroke =
                            x == 0
                                ? new SolidColorBrush(
                                    Color.FromRgb(
                                        120,
                                        120,
                                        120))
                                : new SolidColorBrush(
                                    Color.FromRgb(
                                        55,
                                        60,
                                        67)),

                        StrokeThickness =
                            x == 0 ? 1.5 : 1
                    };

                GridCanvas.Children.Add(line);
            }


            // --------------------------------------------------------
            // LIGNES HORIZONTALES
            // --------------------------------------------------------

            int minY =
                (int)Math.Floor(
                    -centerY / PixelsPerMeter);

            int maxY =
                (int)Math.Ceiling(
                    centerY / PixelsPerMeter);

            for (int y = minY;
                 y <= maxY;
                 y++)
            {
                double screenY =
                    centerY -
                    y * PixelsPerMeter;

                Line line =
                    new Line
                    {
                        X1 = 0,
                        Y1 = screenY,
                        X2 = width,
                        Y2 = screenY,

                        Stroke =
                            y == 0
                                ? new SolidColorBrush(
                                    Color.FromRgb(
                                        120,
                                        120,
                                        120))
                                : new SolidColorBrush(
                                    Color.FromRgb(
                                        55,
                                        60,
                                        67)),

                        StrokeThickness =
                            y == 0 ? 1.5 : 1
                    };

                GridCanvas.Children.Add(line);
            }
        }


        // ============================================================
        // AXES
        // ============================================================

        private void UpdateAxes()
        {
            double width =
                GraphCanvas.ActualWidth;

            double height =
                GraphCanvas.ActualHeight;

            if (width <= 0 ||
                height <= 0)
            {
                return;
            }

            double centerX =
                width / 2.0;

            double centerY =
                height / 2.0;


            // X

            XAxis.X1 = 0;
            XAxis.Y1 = centerY;

            XAxis.X2 = width;
            XAxis.Y2 = centerY;


            // Y

            YAxis.X1 = centerX;
            YAxis.Y1 = 0;

            YAxis.X2 = centerX;
            YAxis.Y2 = height;


            // Labels

            Canvas.SetLeft(
                XAxisLabel,
                width - 25);

            Canvas.SetTop(
                XAxisLabel,
                centerY + 5);

            Canvas.SetLeft(
                YAxisLabel,
                centerX + 5);

            Canvas.SetTop(
                YAxisLabel,
                5);
        }


        // ============================================================
        // REDESSIN
        // ============================================================

        private void Redraw()
        {
            DrawGrid();

            UpdateAxes();

            UpdateRobot();

            UpdateWaypoint();
        }


        // ============================================================
        // RESIZE
        // ============================================================

        private void GraphCanvas_SizeChanged(
            object sender,
            SizeChangedEventArgs e)
        {
            Redraw();
        }


        // ============================================================
        // ZOOM
        // ============================================================

        public void SetZoom(
            double pixelsPerMeter)
        {
            if (pixelsPerMeter < 10)
                pixelsPerMeter = 10;

            if (pixelsPerMeter > 500)
                pixelsPerMeter = 500;

            PixelsPerMeter =
                pixelsPerMeter;

            Redraw();
        }
    }
}