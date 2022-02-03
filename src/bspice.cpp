#include "bspice.hpp"
#include "circuit.hpp"
#include "sim_engine.hpp"
#include "plot.hpp"

/* Simulator variables - The circuit and the simulator engine */
static Circuit circuit_manager;
static simulator_engine sim_manager;

/* For GUI */
#ifdef BSPICE_GUI_ENABLED
#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/stattext.h>

/* Forward declarations */
std::string bspice_error_report(return_codes_e errcode);
static return_codes_e bspice_single_run(int argc, char **argv);

/* Window the GUI is in */
class MainWindow : public wxFrame
{
    public:
        MainWindow(const wxString& title, int width, int height)
        : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(width, height))
        {
            /* Create and add the panel to the frame. */
            m_Panel = new wxPanel(this, wxID_ANY);

            /* Points for buttons */
            wxPoint set_but_point(5,40), load_but_point(5, 165);
            wxPoint run_but_point(5,235), plot_but_point(300, 40);

            /* Points for boxes */
            wxPoint set_box_point(5, 5);
            wxPoint load_box_point(5, 130), plot_box_point(300, 5);

            /* Sizes for boxes */
            wxSize set_box_sz(190, 32), load_box_sz(190, 32);
            wxSize plot_box_sz(190, 32);

            /* Create and add the edit box to the panel. */
            set_box = new wxTextCtrl(m_Panel, wxID_ANY, wxT(""), set_box_point, set_box_sz);
            load_box = new wxTextCtrl(m_Panel, wxID_ANY, wxT(""), load_box_point, load_box_sz);
            plot_box = new wxTextCtrl(m_Panel, wxID_ANY, wxT(""), plot_box_point, plot_box_sz);

            /* Increase font size */
            wxFont font = this->GetFont();
            font.SetPixelSize(wxSize(0,24));
            this->SetFont(font);

            /* Create and add the button to the panel. */
            set_but = new wxButton(m_Panel, wxID_ANY, wxT("Send Command"), set_but_point, wxSize(150, 25));
            set_but->Bind(wxEVT_BUTTON, &MainWindow::set_event, this);
            load_but = new wxButton(m_Panel, wxID_ANY, wxT("Load File"), load_but_point, wxSize(90, 25));
            load_but->Bind(wxEVT_BUTTON, &MainWindow::load_event, this);
            run_but = new wxButton(m_Panel, wxID_ANY, wxT("Run"), run_but_point, wxSize(80, 25));
            run_but->Bind(wxEVT_BUTTON, &MainWindow::run_event, this);
            plot_but = new wxButton(m_Panel, wxID_ANY, wxT("Plot"), plot_but_point, wxSize(80, 25));
            plot_but->Bind(wxEVT_BUTTON, &MainWindow::plot_event, this);

            /* Image */
            image = new wxStaticBitmap( m_Panel, wxID_ANY, wxBitmap("../img/bspice_logo1.png", wxBITMAP_TYPE_PNG), wxPoint(300, 115), wxSize(216, 194));

            /* Center the window on the screen. */
            Centre();
        }

    private:
        wxPanel *m_Panel;

        /* Boxes */
        wxTextCtrl *set_box, *plot_box;
        wxTextCtrl *load_box;

        /* Buttons */
        wxButton *run_but, *set_but;
        wxButton *plot_but, *load_but;

        wxStaticBitmap *image;

        void set_event(wxCommandEvent &event)
        {
            /* Check the input */
            wxString path = set_box->GetValue();
            if(path == "") return; // TODO - handle weird input
            set_box->SetValue(wxString(""));

            /* Checks */
            if(!circuit_manager.valid())
            {
                wxMessageBox(wxString(bspice_error_report(FAIL_GUI_NO_CIRC_LOADED).c_str(), wxConvUTF8) , "Command set", wxOK);
                return;
            }

            /* Format the string in order to match the circuit interface */
            std::string stdline_str(path.mb_str());

            /* Make the update and report */
            return_codes_e errcode = circuit_manager.update(stdline_str);

            /* Report */
            wxMessageBox(wxString(bspice_error_report(errcode).c_str(), wxConvUTF8) , "Command set", wxOK);
        }

        void load_event(wxCommandEvent &event)
        {
            /* Check the input and reset box */
            wxString path = load_box->GetValue();
            if(path == "") return; // TODO - handle weird input
            load_box->SetValue(wxString(""));

            /* Format the string in order to match the circuit interface */
            std::string stdline_str(path.mb_str());

            /* Reinitialize the circuit */
            return_codes_e errcode = circuit_manager.create(stdline_str);

            /* If a different circuit is loaded, we have to clean up the old simulation */
            if(errcode == RETURN_SUCCESS) sim_manager.clear();

            /* Report */
            wxMessageBox(wxString(bspice_error_report(errcode).c_str(), wxConvUTF8) , "Load file", wxOK);
        }

        void run_event(wxCommandEvent &event)
        {
            /* Checks */
            if(!circuit_manager.valid())
            {
                wxMessageBox(wxString(bspice_error_report(FAIL_GUI_NO_CIRC_LOADED).c_str(), wxConvUTF8) , "Run simulation", wxOK);
                return;
            }

            /* Update and run */
            sim_manager.update(circuit_manager);
            return_codes_e errcode = sim_manager.run();

            /* Report */
            wxMessageBox(wxString(bspice_error_report(errcode).c_str(), wxConvUTF8) , "Run simulation", wxOK);
        }

        void plot_event(wxCommandEvent &event) //TODO
        {
            /* Check the input */
            wxString path = plot_box->GetValue();
            if(path == "") return; // TODO - handle weird input
            plot_box->SetValue(wxString(""));

            /* Add the plot option */
            path = "PLOT " + path;

            /* Checks */
            if(!circuit_manager.valid())
            {
                wxMessageBox(wxString(bspice_error_report(FAIL_GUI_NO_SIM_LOADED).c_str(), wxConvUTF8) , "Plot", wxOK);
                return;
            }

            /* Format the string in order to match the circuit interface */
            std::string stdline_str(path.mb_str());

            /* Now make the necessary updates to the circuit and the sim engine before continuing */
            return_codes_e errcode = circuit_manager.update(stdline_str);

            if(errcode != RETURN_SUCCESS)
            {
                wxMessageBox(wxString(bspice_error_report(errcode).c_str(), wxConvUTF8) , "Plot", wxOK);
                return;
            }

            /* Means we run the simulation directly */
            if(circuit_manager.getMemMode() || !sim_manager.valid())
            {
                sim_manager.update(circuit_manager);
                errcode = sim_manager.run();

                if(errcode != RETURN_SUCCESS)
                {
                    wxMessageBox(wxString(bspice_error_report(errcode).c_str(), wxConvUTF8) , "Plot", wxOK);
                    return;
                }
            }
            else
            {
                /* Simple update */
                sim_manager.updatePlotResults(circuit_manager);
            }

            errcode = plot(circuit_manager, sim_manager);
            wxMessageBox(wxString(bspice_error_report(errcode).c_str(), wxConvUTF8) , "Plot", wxOK);
        }
};

/* Application class */
class MyApp : public wxApp
{
    public:
        virtual bool OnInit() // TODO create on exit too
        {
            /* Create and show the application's main window. */
            MainWindow *mainWindow = new MainWindow(wxT("Bspice Simulator v0.1 (2021)"), 640, 480);
            mainWindow->Show(true);

            return true;
        };
};

/*!
    @brief      MACRO that implements the routines for wxEntry from main (GUI).
*/
wxIMPLEMENT_APP_NO_MAIN(MyApp);

#endif

/*!
    @brief      Reports the potential errors using the error code returns,
    during the simulator run.
    @param      errcode  The error code in question.
    @return     The string to print to output
*/
std::string bspice_error_report(return_codes_e errcode)
{
    std::string ret_str = "[ERROR - " + std::to_string(errcode) + "]: ";

    switch(errcode)
    {
        case RETURN_SUCCESS: ret_str = "Success."; break;
        case FAIL_ARG_NUM: ret_str += "Invalid number of input arguments. Syntax is as follows => ./bspice <filename>"; break;

        /* Parser */
        case FAIL_LOADING_FILE: ret_str += "Unable to open input file"; break;
        case FAIL_PARSER_INVALID_FORMAT: ret_str += "Invalid specification in spice netlist."; break;
        case FAIL_PARSER_ELEMENT_EXISTS: ret_str += "Element already exists in spice netlist (SPICE element assertion)."; break;
        case FAIL_PARSER_ELEMENT_NOT_EXISTS: ret_str += "Element does not exist in spice netlist (SPICE card assertion)."; break;
        case FAIL_PARSER_UNKNOWN_ELEMENT: ret_str += "Element type is not supported by the simulator."; break;
        case FAIL_PARSER_SHORTED_ELEMENT: ret_str += "Element is shorted, 2 or more nodes are the same."; break;
        case FAIL_PARSER_UNKNOWN_SPICE_CARD: ret_str += "Uknown spice card option."; break;
        case FAIL_PARSER_SOURCE_SPEC_ARGS: ret_str += "Element source spec uknown or syntax failure."; break;
        case FAIL_PARSER_SOURCE_SPEC_ARGS_NUM: ret_str += "Element source spec number of arguments failure."; break;
        case FAIL_PARSER_SOURCE_SPEC_ARGS_FORMAT: ret_str += "Element source spec syntax failure."; break;
        case FAIL_PARSER_ANALYSIS_INVALID_ARGS: ret_str += "SPICE card invalid arguments or syntax."; break;
        case FAIL_PARSER_UKNOWN_OPTION_OR_REPETITION: ret_str += "SPICE card (.OPTION) uknown option or reinstatiation"; break;

        /* Simulator engine opcodes - Used inside mna/sim_engine.cpp */
        case FAIL_SIMULATOR_RUN: ret_str += "Failure during simulation run."; break;
        case FAIL_SIMULATOR_EMPTY: ret_str += "No circuit is has been loaded, can't run empty simulation."; break;
        case FAIL_SIMULATOR_FACTORIZATION: ret_str += "Failure during factorization (Singular matrix)."; break;
        case FAIL_SIMULATOR_SOLVE: ret_str += "Failure during backwards solving (Solve failure)."; break;

        /* Plotter engine opcodes - Used inside plot.cpp */
        case FAIL_PLOTTER_CIRCUIT_INVALID: ret_str += "No circuit is loaded currently, can't plot."; break;
        case FAIL_PLOTTER_RESULTS_INVALID: ret_str += "Results are not available, need to run simulator in order to plot."; break;
        case FAIL_PLOTTER_NOTHING_TO_PLOT: ret_str += "Failure, nothing to plot."; break;
        case FAIL_PLOTTER_IO_OPERATIONS: ret_str += "Failure in opening plot necessary plot I/O."; break;

        /* GUI commands failure */
        case FAIL_PARSER_EMPTY_COMMAND_GUI: ret_str += "Empty command input."; break;
        case FAIL_GUI_IS_NOT_AVAILABLE: ret_str += "GUI is not supported in the application."; break;
        case FAIL_GUI_NO_CIRC_LOADED: ret_str += "No circuit is loaded currently, to perform command."; break;
        case FAIL_GUI_NO_SIM_LOADED: ret_str += "No simulation has been run in order to plot."; break;

        /* Debug codes, invisible to the user */
        case FAIL_SIMULATOR_FALLTHROUTH_ODE_OPTION: ret_str += "ODE failure, not a known ODE Method."; break;
        default: ret_str = "UKNOWN ERROR"; break; // Nothing, can't reach this point
    }

    return ret_str;
}

/*!
    @brief      The entire simulation run, non-interactive.
    @param      argc The command line process's number of arguments
    @param      argv The command line process's arguments vector
    @return     Error code in case of error, otherwise RETURN_SUCESS
*/
static return_codes_e bspice_single_run(int argc, char **argv)
{
    return_codes_e errcode = RETURN_SUCCESS;
    using namespace std;

    string input_file_name(argv[1]);

    /* Step 2 - Instantiate a circuit, parse and then close the file */
    errcode = circuit_manager.create(input_file_name);
    if(errcode != RETURN_SUCCESS) return errcode;

    /* Step 3 - Proceed to the simulator engine */
    sim_manager.update(circuit_manager);
    errcode = sim_manager.run();
    if(errcode != RETURN_SUCCESS) return errcode;

    /* Step 4 - Output the results */
    return plot(circuit_manager, sim_manager);
}

int main(int argc, char **argv)
{
    using std::cout;
    using std::endl;
    using std::string;

    /* Step 1 - Check for valid number of input arguments */
    if (argc != 2)
    {
        cout << bspice_error_report(FAIL_ARG_NUM) << endl;
        return FAIL_ARG_NUM;
    }

    /* Check whether we have GUI or non-interactive run */
#ifdef BSPICE_GUI_ENABLED
    if(string(argv[1]) == "-gui") return wxEntry(argc, argv);
#else
    if(string(argv[1]) == "-gui")
    {
        cout << bspice_error_report(FAIL_GUI_IS_NOT_AVAILABLE) << endl;
        return FAIL_GUI_IS_NOT_AVAILABLE;
    }
#endif

    /* Non-interactive */
    return_codes_e err = bspice_single_run(argc, argv);
    if(err != RETURN_SUCCESS) cout << bspice_error_report(err) << endl;
    return err;
}
