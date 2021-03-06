#ifndef _SWAY_CONTAINER_H
#define _SWAY_CONTAINER_H
#include <stdint.h>
#include <sys/types.h>
#include <wlr/types/wlr_box.h>
#include <wlr/types/wlr_surface.h>
#include "list.h"
#include "sway/tree/node.h"

struct sway_view;
struct sway_seat;

#define TITLEBAR_BORDER_THICKNESS 1

// Padding includes titlebar border
#define TITLEBAR_H_PADDING 3
#define TITLEBAR_V_PADDING 4

enum sway_container_layout {
	L_NONE,
	L_HORIZ,
	L_VERT,
	L_STACKED,
	L_TABBED,
};

enum sway_container_border {
	B_NONE,
	B_PIXEL,
	B_NORMAL,
};

struct sway_root;
struct sway_output;
struct sway_workspace;
struct sway_view;

enum movement_direction;
enum wlr_direction;

struct sway_container_state {
	// Container properties
	enum sway_container_layout layout;
	double con_x, con_y;
	double con_width, con_height;

	bool is_fullscreen;

	struct sway_workspace *workspace;
	struct sway_container *parent;
	list_t *children;

	struct sway_container *focused_inactive_child;
	bool focused;

	// View properties
	double view_x, view_y;
	double view_width, view_height;

	enum sway_container_border border;
	int border_thickness;
	bool border_top;
	bool border_bottom;
	bool border_left;
	bool border_right;
	bool using_csd;
};

struct sway_container {
	struct sway_node node;
	struct sway_view *view;

	// The pending state is the main container properties, and the current state is in the below struct.
	// This means most places of the code can refer to the main variables (pending state) and it'll just work.
	struct sway_container_state current;

	char *title;           // The view's title (unformatted)
	char *formatted_title; // The title displayed in the title bar

	enum sway_container_layout layout;
	enum sway_container_layout prev_split_layout;

	bool is_sticky;

	// For C_ROOT, this has no meaning
	// For other types, this is the position in layout coordinates
	// Includes borders
	double x, y;
	double width, height;
	double saved_x, saved_y;
	double saved_width, saved_height;

	bool is_fullscreen;

	// The gaps currently applied to the container.
	double current_gaps;
	bool has_gaps;
	double gaps_inner;
	double gaps_outer;

	struct sway_workspace *workspace; // NULL when hidden in the scratchpad
	struct sway_container *parent;    // NULL if container in root of workspace
	list_t *children;                 // struct sway_container

	// Outputs currently being intersected
	list_t *outputs; // struct sway_output

	// Indicates that the container is a scratchpad container.
	// Both hidden and visible scratchpad containers have scratchpad=true.
	// Hidden scratchpad containers have a NULL parent.
	bool scratchpad;

	float alpha;

	struct wlr_texture *title_focused;
	struct wlr_texture *title_focused_inactive;
	struct wlr_texture *title_unfocused;
	struct wlr_texture *title_urgent;
	size_t title_height;
	size_t title_baseline;

	struct {
		struct wl_signal destroy;
	} events;
};

struct sway_container *container_create(struct sway_view *view);

void container_destroy(struct sway_container *con);

void container_begin_destroy(struct sway_container *con);

/**
 * Search a container's descendants a container based on test criteria. Returns
 * the first container that passes the test.
 */
struct sway_container *container_find_child(struct sway_container *container,
		bool (*test)(struct sway_container *view, void *data), void *data);

/**
 * Find a container at the given coordinates. Returns the the surface and
 * surface-local coordinates of the given layout coordinates if the container
 * is a view and the view contains a surface at those coordinates.
 */
struct sway_container *container_at(struct sway_workspace *workspace,
		double lx, double ly, struct wlr_surface **surface,
		double *sx, double *sy);

struct sway_container *tiling_container_at(
		struct sway_node *parent, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy);

void container_for_each_child(struct sway_container *container,
		void (*f)(struct sway_container *container, void *data), void *data);

/**
 * Returns true if the given container is an ancestor of this container.
 */
bool container_has_ancestor(struct sway_container *container,
		struct sway_container *ancestor);

void container_update_textures_recursive(struct sway_container *con);

void container_damage_whole(struct sway_container *container);

void container_reap_empty(struct sway_container *con);

struct sway_container *container_flatten(struct sway_container *container);

void container_update_title_textures(struct sway_container *container);

/**
 * Calculate the container's title_height property.
 */
void container_calculate_title_height(struct sway_container *container);

size_t container_build_representation(enum sway_container_layout layout,
		list_t *children, char *buffer);

void container_update_representation(struct sway_container *container);

/**
 * Return the height of a regular title bar.
 */
size_t container_titlebar_height(void);

/**
 * Resize and center the container in its workspace.
 */
void container_init_floating(struct sway_container *container);

void container_set_floating(struct sway_container *container, bool enable);

void container_set_geometry_from_floating_view(struct sway_container *con);

/**
 * Determine if the given container is itself floating.
 * This will return false for any descendants of a floating container.
 */
bool container_is_floating(struct sway_container *container);

/**
 * Get a container's box in layout coordinates.
 */
void container_get_box(struct sway_container *container, struct wlr_box *box);

/**
 * Move a floating container by the specified amount.
 */
void container_floating_translate(struct sway_container *con,
		double x_amount, double y_amount);

/**
 * Choose an output for the floating container's new position.
 */
struct sway_output *container_floating_find_output(struct sway_container *con);

/**
 * Move a floating container to a new layout-local position.
 */
void container_floating_move_to(struct sway_container *con,
		double lx, double ly);

/**
 * Move a floating container to the center of the workspace.
 */
void container_floating_move_to_center(struct sway_container *con);

bool container_has_urgent_child(struct sway_container *container);

/**
 * If the container is involved in a drag or resize operation via a mouse, this
 * ends the operation.
 */
void container_end_mouse_operation(struct sway_container *container);

void container_set_fullscreen(struct sway_container *container, bool enable);

/**
 * Return true if the container is floating, or a child of a floating split
 * container.
 */
bool container_is_floating_or_child(struct sway_container *container);

/**
 * Return true if the container is fullscreen, or a child of a fullscreen split
 * container.
 */
bool container_is_fullscreen_or_child(struct sway_container *container);

/**
 * Return the output which will be used for scale purposes.
 * This is the most recently entered output.
 */
struct sway_output *container_get_effective_output(struct sway_container *con);

void container_discover_outputs(struct sway_container *con);

void container_remove_gaps(struct sway_container *container);

void container_add_gaps(struct sway_container *container);

enum sway_container_layout container_parent_layout(struct sway_container *con);

enum sway_container_layout container_current_parent_layout(
		struct sway_container *con);

list_t *container_get_siblings(const struct sway_container *container);

int container_sibling_index(const struct sway_container *child);

list_t *container_get_current_siblings(struct sway_container *container);

void container_handle_fullscreen_reparent(struct sway_container *con);

void container_add_child(struct sway_container *parent,
		struct sway_container *child);

void container_insert_child(struct sway_container *parent,
		struct sway_container *child, int i);

void container_add_sibling(struct sway_container *parent,
		struct sway_container *child);

void container_detach(struct sway_container *child);

void container_replace(struct sway_container *container,
		struct sway_container *replacement);

bool sway_dir_to_wlr(enum movement_direction dir, enum wlr_direction *out);

struct sway_container *container_split(struct sway_container *child,
		enum sway_container_layout layout);

#endif
