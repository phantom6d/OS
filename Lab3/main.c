#include <stdio.h>

FILE *inp;
FILE *out;
FILE *backing_store;

int frames_length = 0;
int pages_length = 0;
int tlb_length = 0;
int processed_pages = 0;
int fault_cnt = 0;
int tlb_hit = 0;
int tlb_curr_id = 0;

struct page {
    int offset;
    int page_n;
    int frame_n;
};

struct page tlb [16];
struct page pages [256];
char ram[256][256];

int read_file(struct page *curr_page) {

    if (frames_length >= 256 || pages_length >= 256) {   // проверка, что есть место в таблице страниц
        return -1;
    }

    fseek(backing_store, curr_page->page_n * 256, SEEK_SET);
    fread(ram[frames_length], sizeof(char), 256, backing_store);

    pages[pages_length].page_n = curr_page->page_n;
    pages[pages_length].frame_n = frames_length;
    pages_length++;

    return frames_length++;
}

void insert_tlb(struct page *curr_page) {
    if (tlb_length == 16) {
        tlb_curr_id = (tlb_curr_id + 1) % 16;
        tlb[tlb_curr_id] = *curr_page;
    }
    tlb[tlb_length++] = *curr_page;
}

int get_frame(struct page *curr_page) {

    char hit = 0;

    for (int i = tlb_length; i >= 0; --i) {
        if (tlb[i].page_n == curr_page->page_n) {
            curr_page->frame_n = tlb[i].frame_n;
            tlb_hit++;
            hit = 1;
            break;
        }
    }

    for (int i = 0; i < pages_length; ++i) {
        if (pages[i].page_n == curr_page->page_n) {
            curr_page->frame_n = pages[i].frame_n;
            break;
        }
    }

    if (!hit) {
        curr_page->frame_n = read_file(curr_page);
        fault_cnt++;
    }

    insert_tlb(curr_page);
    return 0;
}

struct page get_page(int log_address) {
    struct page curr_page;
    curr_page.offset = log_address & 0xff;
    curr_page.page_n = (log_address >> 8) & 0xff;
    return curr_page;
}

int main(int argc, char *argv[]) {

    inp = fopen("addresses.txt", "r");
    out = fopen("test_print.txt", "w");
    backing_store = fopen("BACKING_STORE.bin", "rb");

    backing_store = fopen("BACKING_STORE.bin", "rb");
    if (backing_store == NULL) {
        printf("BACKING_STORE.bin reading error\n");
        return 1;
    }

    if (inp == NULL) {
        fprintf(stderr, "address.txt error");
        return -1;
    }

    int logical_add = 0;

    while (fscanf(inp, "%d", &logical_add) == 1) {

        struct page curr_page = get_page(logical_add);
        get_frame(&curr_page);
        fprintf(out, "Virtual address: %d ", (curr_page.page_n << 8)|(curr_page.offset));
        fprintf(out, "Physical address: %d ", (curr_page.frame_n << 8)|(curr_page.offset)); // номер страницы
        fprintf(out, "Value: %d\n", ram[curr_page.frame_n][curr_page.offset]); // оффсет
        processed_pages++;
    }

    printf("\nFault frequency = %0.2f%%\n", fault_cnt * 100. / processed_pages);
    printf("TLB frequency = %0.2f%%\n", tlb_hit * 100. / processed_pages);

    fclose(inp);
    fclose(out);
    fclose(backing_store);
}