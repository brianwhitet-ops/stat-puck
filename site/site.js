(() => {
  const modal = document.getElementById("waitlist-modal");
  const openers = document.querySelectorAll("[data-open-waitlist]");
  const closers = document.querySelectorAll("[data-close-waitlist]");
  const forms = document.querySelectorAll(".waitlist-form");
  const storageKey = "slab-first-batch-interest";
  let lastFocus = null;

  const setStatus = (form, message) => {
    const status = form.parentElement.querySelector("[data-form-status]");
    if (!status) return;
    status.hidden = false;
    status.textContent = message;
  };

  const getFocusable = () =>
    modal.querySelectorAll(
      'button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])'
    );

  const openModal = () => {
    lastFocus = document.activeElement;
    modal.hidden = false;
    document.body.classList.add("modal-open");
    const field = document.getElementById("waitlist-email-modal");
    field?.focus();
  };

  const closeModal = () => {
    modal.hidden = true;
    document.body.classList.remove("modal-open");
    if (lastFocus && typeof lastFocus.focus === "function") {
      lastFocus.focus();
    }
  };

  openers.forEach((button) => {
    button.addEventListener("click", openModal);
  });

  closers.forEach((node) => {
    node.addEventListener("click", closeModal);
  });

  document.addEventListener("keydown", (event) => {
    if (event.key === "Escape" && !modal.hidden) {
      closeModal();
    }

    if (event.key !== "Tab" || modal.hidden) return;
    const focusable = [...getFocusable()];
    if (focusable.length === 0) return;
    const first = focusable[0];
    const last = focusable[focusable.length - 1];
    if (event.shiftKey && document.activeElement === first) {
      event.preventDefault();
      last.focus();
    } else if (!event.shiftKey && document.activeElement === last) {
      event.preventDefault();
      first.focus();
    }
  });

  forms.forEach((form) => {
    form.addEventListener("submit", (event) => {
      event.preventDefault();
      const input = form.querySelector('input[type="email"]');
      const email = (input?.value || "").trim().toLowerCase();
      if (!input?.checkValidity()) {
        input?.reportValidity();
        return;
      }

      try {
        const existing = JSON.parse(localStorage.getItem(storageKey) || "[]");
        const next = Array.isArray(existing) ? existing : [];
        if (!next.includes(email)) next.push(email);
        localStorage.setItem(storageKey, JSON.stringify(next));
      } catch {
        // Review build: still confirm even if storage is blocked.
      }

      if (input) input.value = "";
      setStatus(
        form,
        "You’re on the interest list on this device. We’ll write when a first batch is real."
      );
    });
  });
})();
