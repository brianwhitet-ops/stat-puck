(() => {
  const modal = document.getElementById("waitlist-modal");
  const openers = document.querySelectorAll("[data-open-waitlist]");
  const closers = document.querySelectorAll("[data-close-waitlist]");
  const forms = document.querySelectorAll(".waitlist-form");
  let lastFocus = null;

  const setStatus = (form, message, kind) => {
    const status = form.parentElement.querySelector("[data-form-status]");
    if (!status) return;
    status.hidden = false;
    status.textContent = message;
    status.classList.remove("is-success", "is-error");
    if (kind) status.classList.add(kind);
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
    form.addEventListener("submit", async (event) => {
      event.preventDefault();
      const input = form.querySelector('input[type="email"]');
      const button = form.querySelector('button[type="submit"]');
      const honeypot = form.querySelector('input[name="_gotcha"]');
      const email = (input?.value || "").trim().toLowerCase();

      if (!input?.checkValidity()) {
        input?.reportValidity();
        setStatus(form, "Enter a valid email.", "is-error");
        return;
      }

      const previous = button?.textContent;
      if (button) {
        button.disabled = true;
        button.textContent = "Sending…";
      }

      try {
        const response = await fetch("/api/waitlist", {
          method: "POST",
          headers: { "Content-Type": "application/json", Accept: "application/json" },
          body: JSON.stringify({
            email,
            _gotcha: honeypot?.value || "",
          }),
        });
        const payload = await response.json().catch(() => ({}));
        if (!response.ok || !payload.ok) {
          throw new Error(payload.error || "Could not save that email. Try again.");
        }
        if (input) input.value = "";
        setStatus(
          form,
          "You’re on the first-batch list. We’ll write when a batch is real.",
          "is-success"
        );
      } catch (error) {
        setStatus(
          form,
          error instanceof Error ? error.message : "Could not save that email. Try again.",
          "is-error"
        );
      } finally {
        if (button) {
          button.disabled = false;
          button.textContent = previous || "Notify me";
        }
      }
    });
  });
})();
