/* SPDX-License-Identifier: Zlib */

#include <girara/session.h>
#include <girara/utils.h>
#include <glib/gi18n.h>

#include "document.h"
#include "page.h"
#include "plugin.h"
#include "utils.h"
#include "internal.h"
#include "types.h"

struct zathura_page_s {
  zathura_document_t* document; /**< Parent document */
  void* data;                   /**< Custom data */
  char* label;                  /**< Page label */
  double height;                /**< Page height */
  double width;                 /**< Page width */
  unsigned int index;           /**< Page number */
  bool visible;                 /**< Page is visible */
  bool label_is_number;         /**< Page label is the same as the page number */
};

zathura_page_t* zathura_page_new(zathura_document_t* document, unsigned int index, zathura_error_t* error) {
  if (document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  /* init page */
  zathura_page_t* page = g_try_malloc0(sizeof(zathura_page_t));
  if (page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_ret;
  }

  page->index           = index;
  page->visible         = false;
  page->document        = document;
  page->label_is_number = false;

  /* init plugin */
  const zathura_plugin_t* plugin              = zathura_document_get_plugin(document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);

  zathura_error_t ret = functions->page_init(page);
  if (ret != ZATHURA_ERROR_OK) {
    if (error != NULL) {
      *error = ret;
    }
    goto error_free;
  }

  /* get label if there is one */
  if (functions->page_get_label != NULL) {
    ret = functions->page_get_label(page, page->data, &page->label);
    if (ret != ZATHURA_ERROR_OK) {
      if (error != NULL) {
        *error = ret;
      }
      goto error_free;
    }

    if (page->label != NULL) {
      char page_number_string[G_ASCII_DTOSTR_BUF_SIZE];
      g_ascii_dtostr(page_number_string, G_ASCII_DTOSTR_BUF_SIZE, index + 1);
      page->label_is_number = strcmp(page->label, page_number_string) == 0;
    }
  }

  return page;

error_free:

  if (page != NULL) {
    zathura_page_free(page);
  }

error_ret:

  return NULL;
}

zathura_error_t zathura_page_free(zathura_page_t* page) {
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (page->document == NULL) {
    g_free(page);
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);

  zathura_error_t error = functions->page_clear(page, page->data);

  g_free(page->label);
  g_free(page);

  return error;
}

zathura_document_t* zathura_page_get_document(zathura_page_t* page) {
  if (page == NULL) {
    return NULL;
  }

  return page->document;
}

unsigned int zathura_page_get_index(zathura_page_t* page) {
  if (page == NULL) {
    return 0;
  }

  return page->index;
}

double zathura_page_get_width(zathura_page_t* page) {
  if (page == NULL) {
    return -1;
  }

  return page->width;
}

void zathura_page_set_width(zathura_page_t* page, double width) {
  if (page == NULL) {
    return;
  }

  page->width = width;
}

double zathura_page_get_height(zathura_page_t* page) {
  if (page == NULL) {
    return -1;
  }

  return page->height;
}

void zathura_page_set_height(zathura_page_t* page, double height) {
  if (page == NULL) {
    return;
  }

  page->height = height;
}

bool zathura_page_get_visibility(zathura_page_t* page) {
  if (page == NULL) {
    return false;
  }

  return page->visible;
}

void zathura_page_set_visibility(zathura_page_t* page, bool visibility) {
  if (page == NULL) {
    return;
  }

  page->visible = visibility;
}

void* zathura_page_get_data(zathura_page_t* page) {
  if (page == NULL) {
    return NULL;
  }

  return page->data;
}

void zathura_page_set_data(zathura_page_t* page, void* data) {
  if (page == NULL) {
    return;
  }

  page->data = data;
}

girara_list_t* zathura_page_search_text(zathura_page_t* page, const char* text, zathura_error_t* error) {
  if (page == NULL || page->document == NULL || text == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_search_text == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  return functions->page_search_text(page, page->data, text, error);
}

girara_list_t* zathura_page_links_get(zathura_page_t* page, zathura_error_t* error) {
  if (page == NULL || page->document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_links_get == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  return functions->page_links_get(page, page->data, error);
}

zathura_error_t zathura_page_links_free(girara_list_t* UNUSED(list)) {
  return false;
}

girara_list_t* zathura_page_form_fields_get(zathura_page_t* page, zathura_error_t* error) {
  if (page == NULL || page->document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_form_fields_get == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  return functions->page_form_fields_get(page, page->data, error);
}

zathura_error_t zathura_page_form_fields_free(girara_list_t* UNUSED(list)) {
  return ZATHURA_ERROR_NOT_IMPLEMENTED;
}

girara_list_t* zathura_page_images_get(zathura_page_t* page, zathura_error_t* error) {
  if (page == NULL || page->document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_images_get == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  return functions->page_images_get(page, page->data, error);
}

cairo_surface_t* zathura_page_image_get_cairo(zathura_page_t* page, zathura_image_t* image, zathura_error_t* error) {
  if (page == NULL || page->document == NULL || image == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_image_get_cairo == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  return functions->page_image_get_cairo(page, page->data, image, error);
}

char* zathura_page_get_text(zathura_page_t* page, zathura_rectangle_t rectangle, zathura_error_t* error) {
  if (page == NULL || page->document == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_get_text == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  return functions->page_get_text(page, page->data, rectangle, error);
}

girara_list_t* zathura_page_get_selection(zathura_page_t* page, zathura_rectangle_t rectangle, zathura_error_t* error) {
  if (page == NULL || page->document == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_get_selection == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  return functions->page_get_selection(page, page->data, rectangle, error);
}

zathura_error_t zathura_page_render(zathura_page_t* page, cairo_t* cairo, bool printing) {
  if (page == NULL || page->document == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);

  return functions->page_render_cairo(page, page->data, cairo, printing);
}

const char* zathura_page_get_label(zathura_page_t* page, zathura_error_t* error) {
  if (page == NULL || page->document == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  return page->label;
}

bool zathura_page_label_is_number(zathura_page_t* page) {
  if (page == NULL) {
    return false;
  }

  return page->label_is_number;
}

girara_list_t* zathura_page_get_signatures(zathura_page_t* page, zathura_error_t* error) {
  if (page == NULL || page->document == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  const zathura_plugin_t* plugin              = zathura_document_get_plugin(page->document);
  const zathura_plugin_functions_t* functions = zathura_plugin_get_functions(plugin);
  if (functions->page_get_signatures == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
    }
    return NULL;
  }

  zathura_error_t e  = ZATHURA_ERROR_OK;
  girara_list_t* ret = functions->page_get_signatures(page, page->data, &e);
  if (e != ZATHURA_ERROR_OK) {
    if (error) {
      *error = e;
    }
    girara_list_free(ret);
    return NULL;
  }

  return ret;
}
int
zathura_page_get_region(zathura_page_t* page, zathura_rectangle_t* rectangle, char* buffer)
{
  return snprintf(buffer, 64, "page=%d,trim=%.2fbp %.2fbp %.2fbp %.2fbp,clip",
                  page->index + 1,
                  rectangle->x1,
                  page->height - rectangle->y2,
                  page->width - rectangle->x2,
                  rectangle->y1);
}
