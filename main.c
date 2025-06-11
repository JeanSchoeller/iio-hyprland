#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>
    
// corresponds to hyprctl orientation integers
enum Orientation { Normal, LeftUp, BottomUp, RightUp, Undefined};

DBusError error;
char* output = "eDP-1"; // Default output device
int rotate_master_layout = 0; // Default layout

enum Orientation last_handled_orientation = Undefined;

void dbus_disconnect(DBusConnection* connection) {
    dbus_connection_flush(connection);
    dbus_connection_close(connection);
    dbus_connection_unref(connection);
    dbus_error_free(&error);
}

DBusConnection* dbus_connect(void) {
    dbus_error_init(&error);
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection != NULL) {
        DBusMessage* msg = dbus_message_new_method_call(
            "net.hadess.SensorProxy", "/net/hadess/SensorProxy",
            "net.hadess.SensorProxy", "ClaimAccelerometer");
        char ok = dbus_connection_send_with_reply_and_block(
                      connection, msg, DBUS_TIMEOUT_INFINITE, &error) != NULL;
        dbus_message_unref(msg);
        if (ok) {
            return connection;
        }
    }
    return NULL;
}

enum Orientation property_to_enum(const char* orientation) {
    if (!strcmp(orientation, "normal")) {
        return Normal;
    }
    if (!strcmp(orientation, "bottom-up")) {
        return BottomUp;
    }
    if (!strcmp(orientation, "left-up")) {
        return LeftUp;
    }
    if (!strcmp(orientation, "right-up")) {
        return RightUp;
    }

    return Undefined;
}

enum Orientation parse_orientation_signal(DBusMessage* msg) {
    DBusMessageIter args, iter_array, iter_dict, iter_v;
    const char *iface, *property, *orientation;
    if (dbus_message_iter_init(msg, &args)) {
        dbus_message_iter_get_basic(&args, &iface);
        if (!strcmp("net.hadess.SensorProxy", iface)) {
            dbus_message_iter_next(&args);
            dbus_message_iter_recurse(&args, &iter_array);
            dbus_message_iter_recurse(&iter_array, &iter_dict);
            dbus_message_iter_get_basic(&iter_dict, &property);
            if (!strcmp(property, "AccelerometerOrientation")) {
                dbus_message_iter_next(&iter_dict);
                dbus_message_iter_recurse(&iter_dict, &iter_v);
                dbus_message_iter_get_basic(&iter_v, &orientation);
                return property_to_enum(orientation);
            }
        }
    }
    return Undefined;
}

void system_fmt(char* format, ...) {
    char command[420];
    va_list args;
    va_start(args, format);
    vsnprintf(command, sizeof(command), format, args);
    system(command);
    va_end(args);
}

void handle_orientation(enum Orientation orientation, const char* monitor_id) {
    if (orientation == Undefined || orientation == last_handled_orientation)
        return;

    // Ran if the --either --left-master or --right-master is pass in
    // (pray that our lord and savior vaxry won't change hyprctl output)
    if (rotate_master_layout == 1) {
        if (orientation == Normal) { // --left-master flag
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:left\"", output, orientation, orientation, orientation, monitor_id);
        }
        else if (orientation == LeftUp) {
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:top\"", output, orientation, orientation, orientation, monitor_id);
        }
        else if (orientation == BottomUp) {
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:left\"", output, orientation, orientation, orientation, monitor_id);
        }
        else { // This covers RightUp orientation
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:top\"", output, orientation, orientation, orientation, monitor_id);
        }
    }
    else if (rotate_master_layout == 2) { // --right-master flag
        if (orientation == Normal) {
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:right\"", output, orientation, orientation, orientation, monitor_id);
        }
        else if (orientation == LeftUp) {
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:bottom\"", output, orientation, orientation, orientation, monitor_id);
        }
        else if (orientation == BottomUp) {
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:right\"", output, orientation, orientation, orientation, monitor_id);
        }
        else { // This covers RightUp orientation
            system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d ; keyword workspace m[%s], layoutopt:orientation:bottom\"", output, orientation, orientation, orientation, monitor_id);
        }
    }
    else {
        // Rotates monitor and touch device without changing layout if the --rotate-flag-layout flag is not passed
        system_fmt("hyprctl --batch \"keyword monitor %s,transform,%d ; keyword input:touchdevice:transform %d ; keyword input:tablet:transform %d\"", output, orientation, orientation, orientation);

    }

    last_handled_orientation = orientation;
}

DBusMessage* request_orientation(DBusConnection* conn) {

    // create request calling Get method
    DBusMessage* req = dbus_message_new_method_call(
        "net.hadess.SensorProxy", // destination
        "/net/hadess/SensorProxy", // path
        "org.freedesktop.DBus.Properties", // iface
        "Get" // method
    );

    // append arguments of Get method to request
    const char* interface_name = "net.hadess.SensorProxy";
    const char* property_name = "AccelerometerOrientation";
    dbus_message_append_args(req,
        DBUS_TYPE_STRING, &interface_name,
        DBUS_TYPE_STRING, &property_name,
        DBUS_TYPE_INVALID
    );

    // send request and get reply
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(
        conn,
        req,
        DBUS_TIMEOUT_INFINITE,
        &error
    );

    // check reply
    if (dbus_error_is_set(&error)) {
        printf("Error receiving orientation request: %s: %s\n",
               error.name, error.message);
    }

    // clean up and return
    dbus_message_unref(req);
    return reply;
}

enum Orientation parse_orientation_reply(DBusMessage* reply) {
    DBusMessageIter iter, sub_iter;
    const char* orientation;
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_recurse(&iter, &sub_iter); // DBUS_TYPE_VARIANT
    dbus_message_iter_get_basic(&sub_iter, &orientation); // DBUS_TYPE_STRING
    return property_to_enum(orientation);
}

void init_orientation(DBusConnection* conn, const char* monitor_id) {
// this function proactively requests current orientation, even if not changed
    DBusMessage* reply = request_orientation(conn);
    if (reply != NULL) {
        handle_orientation(parse_orientation_reply(reply), monitor_id);
        dbus_message_unref(reply);
    }
}

void listen_orientation(DBusConnection* connection, const char* monitor_id) {
// this function passively listens for changes to orientation
    DBusMessage* msg;
    dbus_bus_add_match(connection,
        "type='signal',interface='org.freedesktop.DBus.Properties'", &error);
    dbus_bus_add_match(connection,
        "type='signal',sender='org.freedesktop.DBus',interface='org."
        "freedesktop.DBus',member='NameOwnerChanged',arg0='net.hadess."
        "SensorProxy'",
        &error);
    dbus_connection_flush(connection);
    while (dbus_connection_read_write_dispatch(connection, -1)) {
        msg = dbus_connection_pop_message(connection);
        if (msg != NULL) {
            if (dbus_message_is_signal(msg, "org.freedesktop.DBus.Properties",
                    "PropertiesChanged")) {
                if (parse_orientation_signal(msg) != Undefined) {
                    // Handle batches of messages from sensitive sensors
                    usleep(1000 * 300); // 300ms
                    dbus_connection_flush(connection);
                    init_orientation(connection, monitor_id);
                }
            } else {
                dbus_message_unref(msg);
                break;
            }
            dbus_message_unref(msg);
        }
    }
}

char* get_monitor_id(const char* monitor_name) { 
    // Monitor prop for workspace layoutopt rule isn't accepting monitor name, for some reason
    // This is a workaround to get monitor ID instead

    char command[256];
    snprintf(command, sizeof(command), "hyprctl monitors -j all | jq -r '.[] | select(.name==\"%s\") | .id'", monitor_name, monitor_name);
    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen");
        return NULL;
    }

    static char monitor_id[16];
    if (fgets(monitor_id, sizeof(monitor_id), fp) == NULL) {
        perror("fgets");
        pclose(fp);
        return NULL;
    }

    pclose(fp);
    // Remove newline character if present
    monitor_id[strcspn(monitor_id, "\n")] = '\0';
    return monitor_id;
}

int main(int argc, char* argv[]) {
    DBusConnection* connection = dbus_connect();
    if (connection == NULL) {
        printf("error: cannot open dbus connection\n");
        return 1;
    }

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--left-master") == 0) {
            rotate_master_layout = 1; // Enable rotate-layout if flag is found
        }
        else if (strcmp(argv[i], "--right-master") == 0) {
            rotate_master_layout = 2; // Enable rotate-layout if flag is found
        }
        else {
            output = argv[i];
        }
    }

    // Get monitor ID for the specified output
    char* monitor_id = get_monitor_id(output);
    if (monitor_id == NULL) {
        printf("error: cannot get monitor ID for %s\n", output);
        dbus_disconnect(connection);
        return 1;
    }

    // if hyprland and iio-hyprland are restarted after display is already rotated,
    // init_orientation ensures correct immediate orientation without
    // waiting for display to move
    init_orientation(connection, monitor_id);

    listen_orientation(connection, monitor_id);

    dbus_disconnect(connection);
    return 0;
}
