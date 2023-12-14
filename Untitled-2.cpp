#include <iostream>
#include <string>
#include <vector>
#include <chrono>   // Manejo de tiempos
#include <limits>   // Necesario para std::numeric_limits
#include <iomanip>  // Agrega esta línea para std::get_time
#include <sstream>  // Agrega esta línea para std::istringstream
#include <fstream>  // Tratamiento de archivos

// Constantes globales
const std::string NOMBRE_ARCHIVO = "AgendaDB.txt";

// Estructura para representar un evento
struct Evento
{
  std::string titulo;
  std::chrono::system_clock::time_point fechaHora;
};

// *============================================ Funciones Modelo

std::vector<Evento> getEventosDB(){
  std::vector<Evento> agendaArchivo;
  std::string linea;

  std::ifstream archivo(NOMBRE_ARCHIVO);
  if ( archivo.is_open() )
  {
    while (std::getline(archivo, linea)) {
      std::string fechaHoraStr;
      Evento eventoLeido;

      std::istringstream iss(linea);
      std::getline(iss, eventoLeido.titulo, ',');
      std::getline(iss, fechaHoraStr); // Resto de la línea es la fecha y hora

      std::tm tm = {};
      std::istringstream issFechaHora(fechaHoraStr);
      issFechaHora >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
      eventoLeido.fechaHora = std::chrono::system_clock::from_time_t(std::mktime(&tm));
      agendaArchivo.push_back(eventoLeido);
    }
  
    return agendaArchivo;
    
  } else {
    std::cerr << "Parece que es la primera vez que esta con nosotros, puede comenzar a utilizar su agenda personal" << std::endl;
    return agendaArchivo;
  }

  
}

void setEventosDB( std::vector<Evento>agenda ){
  // ~ Borrar el contenido del archivo
  std::ofstream archivo(NOMBRE_ARCHIVO, std::ofstream::trunc);

  // ~ Setear los valores
  for (const Evento& evento : agenda){
    if (archivo.is_open()) {

      std::time_t tiempoEvento = std::chrono::system_clock::to_time_t(evento.fechaHora);

      archivo << evento.titulo << "," << std::put_time(std::localtime(&tiempoEvento), "%Y-%m-%d %H:%M:%S") << std::endl;
    }
  }
  archivo.close();
}


// *============================================ Funciones de ayuda
double pedirNumero()
{

  double numero{ 0 };
  // Valida que sea un numero, diferente a cero y no contenga nada despues
  do
  {
    if (std::cin >> numero && std::cin.peek() == '\n')
    {
      std::cin.ignore(); // Limpiar el buffer despues de obtener el numero
      return numero;
    }
    else
    {
      // Limpia el cin y sus errores para pedir otra vez el numero
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Ingresa solamente un numero valido por favor." << std::endl;
    }
  } while (true);

}

auto getFechaActual(int& horasDiferencia){
  // Configuración de la zona horaria
  std::chrono::hours offset(horasDiferencia);
  auto zonaHoraria = std::chrono::system_clock::time_point::duration(offset);

  // Aplicar la zona horaria a la hora actual
  auto ahora = std::chrono::system_clock::now();
  auto ahoraConZona = ahora + zonaHoraria;
  return ahoraConZona;
}

// *============================================ Operaciones de la agenda
// Función para listar los eventos y mostrar cuanto falta para cada uno
void verEventos(std::vector<Evento>& agenda, int& horasDiferencia)
{
  // Fecha actual
  auto ahora = getFechaActual(horasDiferencia); // Se utiliza auto para que c++ le asigne automaticamente el tipo
  bool hayEventos = false;

  std::cout << "\n\n\n" << std::endl;
  std::cout << "Eventos en tu agenda:" << std::endl;

  // Recorrer agneda
  for (const Evento& evento : agenda)
  {
    std::chrono::duration<double> duracion = evento.fechaHora - ahora;
    // Si el evento esta en el futuro...
    if (duracion.count() > 0)
    {
      std::time_t tiempoEvento = std::chrono::system_clock::to_time_t(evento.fechaHora);
      std::cout << "Título: " << evento.titulo << std::endl;
      std::cout << "Fecha y hora: " << std::put_time(std::localtime(&tiempoEvento), "%Y-%m-%d %H:%M:%S") << std::endl;

      // Calcular los tiempo restante
      int dias = std::chrono::duration_cast<std::chrono::hours>(duracion).count() / 24;
      int horas = std::chrono::duration_cast<std::chrono::hours>(duracion).count() % 24;
      int minutos = std::chrono::duration_cast<std::chrono::minutes>(duracion).count() % 60;
      int segundos = std::chrono::duration_cast<std::chrono::seconds>(duracion).count() % 60;

      std::cout << "Tiempo restante: " << dias << " días, " << horas << " horas, " << minutos << " minutos, " << segundos << " segundos" << std::endl;
      std::cout << "------------------------" << std::endl;
      hayEventos = true; // setea en true, representa si se imprimio algo o no
    }
  }
  if ( !hayEventos)
  {
    std::cout << "No tienes un evento proximo" << std::endl;
  }
  
  std::cout << "Presiona enter para seguir...";
  std::string prueba;
  std::getline(std::cin, prueba);
}

// Función para agregar un evento a la agenda
void agregarEvento(std::vector<Evento>& agenda)
{
  Evento nuevoEvento;

  std::cout << "Ingresa el título del evento: ";
  std::getline(std::cin, nuevoEvento.titulo);

  do
  {
    std::cout << "Ingresa la fecha y hora del evento en el formato YYYY-MM-DD HH:MM:SS (Hora opcional): ";
    std::string fechaHoraStr;
    std::getline(std::cin, fechaHoraStr);

    // Convertir la cadena de fecha y hora a std::chrono::system_clock::time_point
    try
    {
      std::tm tm = {};
      std::istringstream iss(fechaHoraStr);
      iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
      if (iss.fail())
      {
        throw std::runtime_error("Formato de fecha y hora inválido.");
      }

      nuevoEvento.fechaHora = std::chrono::system_clock::from_time_t(std::mktime(&tm));
      agenda.push_back(nuevoEvento);
      std::cout << "\nEvento agregado con éxito." << std::endl;

      // Guardar el nuevo evento en el archivo
      std::ofstream archivo(NOMBRE_ARCHIVO, std::ios::app);
      if (archivo.is_open()) {
        archivo << nuevoEvento.titulo << "," << fechaHoraStr << std::endl;
        archivo.close();
      } else {
        std::cerr << "No se pudo abrir el archivo para escribir." << std::endl;
      }
      break; // Rompe la ejecucion de la funcion
    }
      catch (const std::exception& e)
      {
        std::cerr << "Error: " << e.what() << std::endl;
      }

  } while (true);
}

// Función para eliminar un evento de la agenda
void eliminarEvento(std::vector<Evento>& agenda)
{

  // Si la agenda esta vacia muestra y sale
  if (agenda.empty())
  {
    std::cout << "No hay eventos en la agenda." << std::endl;
    return;
  }

  // Mostrar la lista de eventos
  std::cout << "Lista de eventos en tu agenda:" << std::endl;
  for (size_t i = 0; i < agenda.size(); ++i)
  {
    std::cout << i + 1 << ". " << agenda[i].titulo << std::endl;
  }

  // Pedir al usuario que seleccione el evento a eliminar
  std::cout << "Seleccione el número del evento que desea eliminar (0 para cancelar): ";
  int opcion = pedirNumero();

  if (opcion >= 1 && static_cast<size_t>(opcion) <= agenda.size())
  {
    // Eliminar el evento seleccionado
    agenda.erase(agenda.begin() + opcion - 1);
    std::cout << "Evento eliminado con éxito." << std::endl;
    setEventosDB(agenda);
  }
  else if (opcion != 0)
  {
    std::cout << "Opción no válida." << std::endl;
  }
}

// Función para actualizar un evento existente
void actualizarEvento(std::vector<Evento>& agenda) {
    std::cout << "Lista de eventos en tu agenda:" << std::endl;

    // Mostrar la lista de eventos
    for (size_t i = 0; i < agenda.size(); ++i) {
        std::cout << i + 1 << ". " << agenda[i].titulo << std::endl;
    }

    // Pedir al usuario que seleccione el evento a actualizar
    std::cout << "Seleccione el número del evento que desea actualizar (0 para cancelar): ";
    int opcion;
    opcion = pedirNumero();

    if (opcion >= 1 && static_cast<size_t>(opcion) <= agenda.size()) {
        Evento& eventoActualizando = agenda[opcion - 1];

        std::cout << "Actualizando el evento '" << eventoActualizando.titulo << "':" << std::endl;

        // Solicitar al usuario la nueva información para el evento
        std::cout << "Nuevo título: ";
        std::getline(std::cin, eventoActualizando.titulo);

        do
        {
          std::cout << "Nueva fecha y hora con formato YYYY-MM-DD HH:MM:SS (Hora opcional): ";
          std::string fechaHoraStr;
          std::getline(std::cin, fechaHoraStr);

          // Convertir la cadena de fecha y hora a std::chrono::system_clock::time_point
          try {
              std::tm tm = {};
              std::istringstream iss(fechaHoraStr);
              iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
              if (iss.fail()) {
                  throw std::runtime_error("Formato de fecha y hora inválido.");
              }
              eventoActualizando.fechaHora = std::chrono::system_clock::from_time_t(std::mktime(&tm));
              std::cout << "Evento actualizado con éxito." << std::endl;
              setEventosDB(agenda);
              break; // Rompe la funcion del do while
          } catch (const std::exception& e) {
              std::cerr << "Error: " << e.what() << std::endl;
          }
        } while (true);
      
    } else if (opcion != 0) {
        std::cout << "Opción no válida." << std::endl;
    }
}

// Muestra los eventos proximos a cumplirse
void mostrarEventosProximos(std::vector<Evento>& agenda, int& horasDiferencia) {

  auto ahora = getFechaActual(horasDiferencia); // Se utiliza auto para que c++ le asigne automaticamente el tipo
  bool hayEventos = false;

  std::cout << "Eventos en tu agenda:" << std::endl;

  // Recorrer agneda
  for (const Evento& evento : agenda)
  {
    std::chrono::duration<double> duracion = evento.fechaHora - ahora;
    // Si el evento esta en el futuro...
    if (duracion.count() > 0 && duracion.count() < 1209600) // Verifica si esta dentro de las siguientes dos semanas, la cantidad es en segundos
    {
      std::time_t tiempoEvento = std::chrono::system_clock::to_time_t(evento.fechaHora);
      std::cout << "Título: " << evento.titulo << std::endl;
      std::cout << "Fecha y hora: " << std::put_time(std::localtime(&tiempoEvento), "%Y-%m-%d %H:%M:%S") << std::endl;

      // Calcular los tiempo restante
      int dias = std::chrono::duration_cast<std::chrono::hours>(duracion).count() / 24;
      int horas = std::chrono::duration_cast<std::chrono::hours>(duracion).count() % 24;
      int minutos = std::chrono::duration_cast<std::chrono::minutes>(duracion).count() % 60;
      int segundos = std::chrono::duration_cast<std::chrono::seconds>(duracion).count() % 60;

      std::cout << "Tiempo restante: " << dias << " días, " << horas << " horas, " << minutos << " minutos, " << segundos << " segundos" << std::endl;
      std::cout << "------------------------" << std::endl;
      hayEventos = true; // setea en true, representa si se imprimio algo o no
    }
  }
  if ( !hayEventos)
  {
    std::cout << "No tienes eventos proximos" << std::endl;
  }
  
  std::cout << "Presiona enter para seguir...";
  std::string prueba;
  std::getline(std::cin, prueba); 
}

int main()
{

  int horasDiferencia { 0 };
 
  auto ahora = getFechaActual(horasDiferencia);
  std::time_t tiempoEvento = std::chrono::system_clock::to_time_t(ahora);


  // Configuración de la zona horaria
  std::cout << "Antes de iniciar necesito saber tu zona horaria." << std::endl;
  std::cout << "Mi fecha actual es " << std::put_time(std::localtime(&tiempoEvento), "%Y-%m-%d %H:%M:%S") << std::endl;
  std::cout << "Cuantas horas hay de diferencia entre mi fecha y la tuya? (Toma en cuenta la diferencia de mi hora a la tuya, este numero puede ser negativo)" << std::endl;
  horasDiferencia = pedirNumero();
  std::cout << "Zona horaria obtenida con exito" << std::endl;


  std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n" << std::endl;

    
  // Declaracion de variables
  std::vector<Evento> agenda {getEventosDB()};


  int opcion_usuario{ 0 };
  

  // *=============================================
  // * Declarar un evento para pruebas

  // titulo
  Evento eventoPrueba;
  eventoPrueba.titulo = "Evento de prueba";
  // tiempo
  std::tm tm = {};
  std::istringstream iss("2023-10-01 23:58:00");                 // istringstream es una clase, la llamamos iss y pasamos la fecha
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  eventoPrueba.fechaHora = std::chrono::system_clock::from_time_t(std::mktime(&tm));
  // agregarEvento
  agenda.push_back(eventoPrueba);



  // *============================================

  verEventos(agenda, horasDiferencia); // Muestra eventos que se cumpliran de aqui a dos semanas

  while (true)
  {
    std::cout << "\n\n"
      << std::endl;
    std::cout << "********************************" << std::endl;
    std::cout << "**** Bienvenido a tu agenda ****" << std::endl;
    std::cout << "********************************" << std::endl;

    std::cout << "Que opcion deseas realizar?" << std::endl;
    std::cout << "1.- Ver eventos" << std::endl;
    std::cout << "2.- Agrega un evento" << std::endl;
    std::cout << "3.- Elimina un evento" << std::endl;
    std::cout << "4.- Actualizar un evento" << std::endl;
    std::cout << "5.- Salir" << std::endl;
    opcion_usuario = pedirNumero();

    switch (opcion_usuario)
    {
    case 1:
      verEventos(agenda, horasDiferencia);
      break;
    case 2:
      agregarEvento(agenda);
      break;
    case 3:
      eliminarEvento(agenda);
      break;
    case 4:
      actualizarEvento(agenda);
      break;
    case 5:
      std::cout << "Nos vemos! :)" << std::endl;
      return 0;
      break;

    default:
      break;
    }
  }

  return 0;
}
